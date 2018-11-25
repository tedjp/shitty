#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <queue>
#include <unordered_map>

#include "Connection.h"
#include "ConnectionManager.h"
#include "Error.h"
#include "EventReceiver.h"
#include "RequestRouter.h"
#include "Server.h"
#include "http1/ServerTransport.h"

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

    int epollFD();

private:
    void setup();
    void loop();
    void dispatch(struct epoll_event *event);
    bool accept();

    void cleanup();
    void close_all_clients();
    void remove_dead_clients();

    Server *server_ = nullptr;

    int epfd_ = -1;

    int listenfd_ = -1;

    RequestRouter request_router_;

    // XXX: Due to the way EventReceivers can't be moved (or the event pointer
    // dangles), they have to be held by unique_ptr.
    std::unordered_map<int, std::unique_ptr<Connection>> clients_;

    std::queue<int> remove_clients_;
};

Server::Server():
    impl_(std::make_unique<Impl>(this))
{}

void Server::run() {
    impl_->run();
}

int Server::epollFD() {
    return impl_->epollFD();
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

        remove_dead_clients();
    }

    int epoll_errno = errno;
    cleanup();
    errno = epoll_errno;

    throw error_errno("epoll_wait");
}

void Server::Impl::remove_dead_clients() {
    while (!remove_clients_.empty()) {
        int fd = remove_clients_.front();
        ::close(fd);
        clients_.erase(fd);
        remove_clients_.pop();
    }
}

void Server::Impl::cleanup() {
    close(listenfd_);
    listenfd_ = -1;
    close_all_clients();
}

void Server::Impl::close_all_clients() {
    clients_.clear();
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

    auto connection = std::make_unique<Connection>(epfd_, client_fd);
    connection->setConnectionManager(this);
    connection->setTransport(std::make_unique<http1::ServerTransport>(
                connection.get(),
                &request_router_));

    auto [iter, inserted] = clients_.try_emplace(client_fd, std::move(connection));

    if (inserted == false)
        throw std::logic_error("Accepted new connection with existing fd");

    // Go straight into client-receive to take advantage of TCP Fast Open or
    // TCP_DELAY_ACCEPT.
    // XXX: Since the connection is already subscribed it'll get an EPOLLIN
    // event that we've already handled. If we swap the order of this immediate
    // read with the subscribe call, it'll avoid that.
    Connection& conn = *iter->second;
    conn.onPollIn();

    return true;
}

void Server::Impl::removeConnection(int fd) {
    remove_clients_.emplace(fd);
}

int Server::Impl::epollFD() {
    return epfd_;
}

} // namespace
