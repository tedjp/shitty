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
#include "Server.h"
#include "SignalReceiver.h"
#include "SwitchTransport.h"

namespace shitty {

class Server::Impl final:
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
    void bind();
    void addEventReceiver(int fd, EventReceiver *receiver);
    void loop();
    void dispatch(struct epoll_event *event);
    bool accept();

    void cleanup();
    void close_all_clients();
    void remove_dead_clients();

    Server *server_ = nullptr;

    int epfd_ = -1;

    int listenfd_ = -1;

    // XXX: Due to the way EventReceivers can't be moved (or the event pointer
    // dangles), they have to be held by unique_ptr.
    std::unordered_map<int, std::unique_ptr<Connection>> clients_;

    std::queue<int> remove_clients_;

    bool keep_running_ = true;
    SignalReceiver signal_receiver_;
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
    server_(server),
    signal_receiver_(SIGINT, [&flag = keep_running_](int signum) { flag = false; })
{
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epfd_ == -1)
        throw error_errno("epoll_create1 failed");

    addEventReceiver(signal_receiver_.getPollFD(), &signal_receiver_);
}

Server::Impl::~Impl() {
    close_all_clients();
    close(epfd_);
    close(listenfd_);
}

void Server::Impl::run() {
    setup();
    loop();
}

static void tcp_defer_accept(int sock) {
    const int secs = 5;
    if (setsockopt(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT, &secs, sizeof(secs)) == -1)
        perror("(this is harmless) TCP_DEFER_ACCEPT");
}

void Server::Impl::setup() {
    listenfd_ = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listenfd_ == -1)
        throw error_errno("socket");

    tcp_defer_accept(listenfd_);
    bind();

    if (::listen(listenfd_, 100) == -1)
        throw error_errno("listen");

    addEventReceiver(listenfd_, this);
}

void Server::Impl::bind()
{
    const std::array try_ports = std::to_array<const uint16_t>({ 80, 8080 });

    struct sockaddr_in6 sin6 = {};
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = in6addr_any;

    for (uint16_t port : try_ports)
    {
        sin6.sin6_port = htons(port);

        if (::bind(
            listenfd_,
            reinterpret_cast<const struct sockaddr*>(&sin6),
            sizeof(sin6)) == 0)
        {
            return;
        }
    }

    throw error_errno("bind: no ports available");
}

void Server::Impl::addEventReceiver(int fd, EventReceiver* receiver) {
    struct epoll_event ev = {
        .events = EPOLLIN | EPOLLET,
        .data = {
            .ptr = receiver,
        },
    };

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
        throw error_errno("EPOLL_CTL_ADD");
}

void Server::Impl::loop() {
    struct epoll_event events[10];
    int count;

    while (keep_running_
            && (
                (count = epoll_wait(epfd_, events, sizeof(events) / sizeof(events[0]), -1)) != -1
                || errno == EINTR))
    {
        for (int i = 0; i < count; ++i)
            dispatch(&events[i]);

        remove_dead_clients();
    }

    // distinguish clean shutdown by epoll_errno
    const int epoll_errno = keep_running_ ? errno : 0;
    cleanup();

    if (epoll_errno != 0)
        throw error_errno("epoll_wait", epoll_errno);
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
    connection->setTransport(std::make_unique<SwitchTransport>(*connection, server_->routes_));

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
