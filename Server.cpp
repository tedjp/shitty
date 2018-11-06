#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <unordered_set>

#include "Connection.h"
#include "Error.h"
#include "EventReceiver.h"
#include "RequestRouter.h"
#include "Server.h"

namespace shitty {

class Server::Impl: public EventReceiver {
public:
    Impl(Server *server);
    ~Impl();

    void run();

    int getPollFD() const override;
    void onPollIn() override;

private:
    void setup();
    void loop();
    void dispatch(struct epoll_event *event);
    bool accept();
    void close_all_clients();

    Server *server_ = nullptr;

    int epfd_ = -1;

    int listenfd_ = -1;

    struct event_receiver {
        enum { EV_LISTENER, EV_CONNECTION } type;
        union {
            int listener_fd;
            Connection *connection;
        };
    };

    RequestRouter request_router_;

    //std::unordered_set<Connection> clients_;
    // Set element must be const.
    std::unordered_set<std::unique_ptr<Connection>> clients_;
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
            .ptr = this,
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
    close_all_clients();
    errno = epoll_errno;

    throw error_errno("epoll_wait");
}

void Server::Impl::close_all_clients() {
    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        (*it)->close();
        clients_.erase(it);
    }
}

void Server::Impl::dispatch(struct epoll_event *event) {
    auto target = reinterpret_cast<EventReceiver*>(event->data.ptr);

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

    // TODO: Subscribe FD
    auto inserted = clients_.emplace(std::make_unique<Connection>(epfd_, client_fd, &request_router_));

    // Go straight into client-receive to take advantage of TCP Fast Open or
    // TCP_DELAY_ACCEPT.
    (*inserted.first)->onPollIn();

    return true;
}

} // namespace
