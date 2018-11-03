#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>

#include "error.h"
#include "server.h"

namespace shitty {

class Server::Impl {
public:
    Impl(Server *server);
    ~Impl();

    void run();

private:
    void setup();
    void loop();
    void dispatch(struct epoll_event *event);

    Server *server_ = nullptr;
    int epfd_ = -1;
    int listenfd_ = -1;
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
    setup();
    loop();
}

void Server::Impl::setup() {
    listenfd_ = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listenfd_ == -1)
        throw error_errno("socket");

    struct sockaddr_in6 sin6 = {};
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(80);
    sin6.sin6_addr = in6addr_any;

    // TODO: TCP_DEFER_ACCEPT

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
        .events = EPOLLIN, // TODO: EPOLLET when looping accept()
        .data = {
            .fd = listenfd_,
        },
    };

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, listenfd_, &ev) == -1)
       throw error_errno("EPOLL_CTL_ADD");
}

void Server::Impl::loop() {
    struct epoll_event events[10];
    int count;

    while ((count = epoll_wait(epfd_, events, sizeof(events) / sizeof(events[0]), -1)) != -1) {
        for (int i = 0; i < count; ++i)
            dispatch(&events[i]);
    }

    throw error_errno("epoll_wait");
}

void Server::Impl::dispatch(struct epoll_event *event) {
    // TODO: Do this repeatedly until EAGAIN/EWOULDBLOCK,
    // then switch to EPOLLET on the listen_ socket.
    int client_fd = accept4(
            event->data.fd,
            nullptr,
            nullptr,
            SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (client_fd == -1) {
        perror("accept4");
        return;
    }

    // Placeholder.
    const char msg[] =
        "HTTP/1.1 200 OK\r\n"
        "Server: Shitty\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain; charset=us-ascii\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "Hello.\n";
    send(client_fd, msg, sizeof(msg) - 1, MSG_DONTWAIT);
    close(client_fd);
}

} // namespace
