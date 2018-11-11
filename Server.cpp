#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <unordered_map>

#include "Connection.h"
#include "ConnectionManager.h"
#include "Error.h"
#include "EventReceiver.h"
#include "RequestRouter.h"
#include "Server.h"

namespace shitty {

class Server::Impl:
    public ConnectionManager,
    public EventReceiver
{
public:
    Impl(Server *server);
    ~Impl();

    void run();

    void removeConnection(int fd) override;

    int getPollFD() const override;
    void onPollIn() override;

private:
    void setup();
    void loop();
    void dispatch(struct epoll_event *event);
    bool accept();

    void cleanup();
    void close_all_clients();

    Server *server_ = nullptr;

    int epfd_ = -1;

    int listenfd_ = -1;

    RequestRouter request_router_;

    // XXX: Due to the way EventReceivers can't be moved (or the event pointer
    // dangles), they have to be held by unique_ptr.
    std::unordered_map<int, std::unique_ptr<Connection>> clients_;
};

Server::Server():
    impl_(std::make_unique<Impl>(this))
{}

void Server::run() {
    impl_->run();
}

Server::~Server() {
}

Server::Impl::Impl(Server *server):
    server_(server)
{
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epfd_ == -1)
        throw error_errno("epoll_create1 failed");
}

Server::Impl::~Impl() {
    close_all_clients();
    close(epfd_);
    close(listenfd_);
}

void Server::Impl::run() {
    request_router_ = RequestRouter(&server_->routes_);
    setup();
    loop();
}

static void tcp_defer_accept(int sock) {
    const int secs = 5;
    if (setsockopt(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT, &secs, sizeof(secs)) == -1)
        perror("TCP_DEFER_ACCEPT");
}

void Server::Impl::setup() {
    listenfd_ = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listenfd_ == -1)
        throw error_errno("socket");

    struct sockaddr_in6 sin6 = {};
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(80);
    sin6.sin6_addr = in6addr_any;

    tcp_defer_accept(listenfd_);

    if (::bind(
                listenfd_,
                reinterpret_cast<const struct sockaddr*>(&sin6),
                sizeof(sin6)) == -1)
    {
        throw error_errno("bind");
    }

    if (::listen(listenfd_, 100) == -1)
        throw error_errno("listen");

    struct epoll_event ev = {
        .events = EPOLLIN | EPOLLET,
        .data = {
            .ptr = dynamic_cast<EventReceiver*>(this),
        },
    };

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, listenfd_, &ev) == -1)
       throw error_errno("EPOLL_CTL_ADD");
}

void Server::Impl::loop() {
    struct epoll_event events[10];
    int count;

    while ((count = epoll_wait(epfd_, events, sizeof(events) / sizeof(events[0]), -1)) != -1 || errno == EINTR) {
        for (int i = 0; i < count; ++i)
            dispatch(&events[i]);
    }

    int epoll_errno = errno;
    cleanup();
    errno = epoll_errno;

    throw error_errno("epoll_wait");
}

void Server::Impl::cleanup() {
    close(listenfd_);
    listenfd_ = -1;
    close_all_clients();
}

void Server::Impl::close_all_clients() {
    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        Connection& conn = *it->second;
        conn.close();
        clients_.erase(it);
    }
}

void Server::Impl::dispatch(struct epoll_event *event) {
    auto target_static_dispatch = reinterpret_cast<EventReceiver*>(event->data.ptr);
    auto target = dynamic_cast<EventReceiver*>(target_static_dispatch);

    // XXX: Beware that the target might be called for multiple events.
    // That means you cannot delete your receiver during an event callback
    // or you'll crash. You have to queue it for cleanup after the list of
    // events has been processed.
    if (event->events & EPOLLOUT)
        target->onPollOut();
    if (event->events & EPOLLIN)
        target->onPollIn();
    if (event->events & EPOLLERR)
        target->onPollErr();
}

int Server::Impl::getPollFD() const {
    return listenfd_;
}

void Server::Impl::onPollIn() {
    while (accept())
        ;
}

// Returns true if a connection was accepted or false if one was not.
bool Server::Impl::accept() {
    int client_fd = accept4(listenfd_, nullptr, nullptr,
            SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (client_fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            throw error_errno("accept4");

        return false;
    }

    auto connection_ptr = std::make_unique<Connection>(epfd_, client_fd, &request_router_);
    auto inserted = clients_.try_emplace(client_fd, std::move(connection_ptr));
    if (inserted.second == false) {
        // if connection becomes movable again this needs to be restored.
        //close(client_fd);
        return true; // connection was still accepted, briefly
    }

    auto& connection = *(*inserted.first).second;
    connection.setConnectionManager(this);

    // Go straight into client-receive to take advantage of TCP Fast Open or
    // TCP_DELAY_ACCEPT.
    connection.onPollIn();

    return true;
}

void Server::Impl::removeConnection(int fd) {
    // Might already have been removed if initiated by close_all_clients().
    clients_.erase(fd);
}

} // namespace
