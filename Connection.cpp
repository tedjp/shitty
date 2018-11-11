#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Connection.h"
#include "Error.h"
#include "HTTP1Transport.h"

using shitty::Connection;

Connection::Connection(int epfd, int fd, RequestRouter *request_router):
    EventReceiver(),
    fd_(fd),
    epfd_(epfd),
    transport_(std::make_unique<HTTP1Transport>(this, request_router)),
    manager_(nullptr)
{
    if (epfd_ < 0)
        throw std::invalid_argument("Bad event FD");

    if (fd_ < 0)
        throw std::invalid_argument("Connection fd invalid");

    subscribe_to_input();
}

Connection::~Connection() {
    try {
        close();
    } catch (...)
    {}
}

void Connection::subscribe_to_input() {
    struct epoll_event events = {
        .events = EPOLLIN | EPOLLET,
        .data = {
            .ptr = dynamic_cast<EventReceiver*>(this),
        }
    };

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &events) == -1)
        throw error_errno("Connection EPOLL_CTL_ADD");
}

void Connection::updateSubscription() {
    struct epoll_event event = {
        .events = EPOLLIN | EPOLLET,
        .data = {
            .ptr = dynamic_cast<EventReceiver*>(this),
        },
    };

    if (!outgoing_.empty())
        event.events |= EPOLLOUT;

    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd_, &event) == -1)
        throw error_errno("Failed to modify connection's epoll");
}

int Connection::getPollFD() const {
    return fd_;
}

void Connection::onPollIn() {
    bool read_something = false;

    for (;;) {
        incoming_.grow();

        ssize_t read_len = ::read(
                fd_,
                incoming_.tail(),
                incoming_.tailroom());

        if (read_len > 0) {
            read_something = true;
            incoming_.addTailContent(static_cast<size_t>(read_len));
        }

        if (read_len == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            close();
            return;
        }

        if (read_len == 0 && !read_something) {
            close();
            return;
        }
    }

    if (read_something)
        transport_->onInput(incoming_);
}

void Connection::close() {
    if (fd_ == -1)
        return;

    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd_, nullptr);
    epfd_ = -1;

    if (manager_)
        manager_->removeConnection(fd_);

    ::close(fd_);
    fd_ = -1;
}

void Connection::onPollOut() {
    if (outgoing_.empty()) {
        updateSubscription();
        return;
    }

    ssize_t sent;
    do {
        sent = ::send(fd_, outgoing_.data(), outgoing_.size(), 0);
        if (sent > 0)
            outgoing_.advance(static_cast<size_t>(sent));
    } while (!outgoing_.empty() && sent > 0);

    if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        close();
        return;
    }

    if (outgoing_.empty())
        updateSubscription();
}

void Connection::setConnectionManager(ConnectionManager* manager) {
    manager_ = manager;
}

void Connection::send(const void *data, size_t len) {
    // FIXME: Queue unsent outgoing data
    ::send(fd_, data, len, 0);
}

void Connection::flush() {
    if (outgoing_.empty())
        return;

    ssize_t sent;

    do {
        sent = ::send(fd_, outgoing_.data(), outgoing_.size(), 0);

        if (sent == 0) {
            close();
            return;
        }

        if (sent < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                close();
                return;
            }
        }

        if (sent > 0)
            outgoing_.advance(static_cast<size_t>(sent));
    } while (sent > 0 && !outgoing_.empty());

    updateSubscription();
}

shitty::StreamBuf& Connection::outgoingStreamBuf() {
    return outgoing_;
}
