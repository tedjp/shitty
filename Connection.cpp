#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Connection.h"
#include "Error.h"
#include "http1/ClientTransport.h"
#include "http1/ServerTransport.h"

using shitty::Connection;

Connection::Connection(
        int epfd,
        int fd,
        shitty::ServerTransport::req_handler_t&& handler):
    EventReceiver(),
    fd_(fd),
    epfd_(epfd),
    transport_(std::make_unique<shitty::http1::ServerTransport>(this, std::move(handler))),
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
    else
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

void Connection::send(const char *data, size_t len) {
    if (outgoing_.empty())
        send_immediate(data, len);
    else
        queue_and_send(data, len);
}

void Connection::send_immediate(const char *data, size_t len) {
    ssize_t sent;
    size_t total_sent = 0;

    do {
        sent = ::send(fd_, data + total_sent, len + total_sent, 0);
        if (sent > 0)
            total_sent += sent;
    } while (sent > 0 && total_sent < len);

    if (sent == 0 || (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        close();
        return;
    }

    if (total_sent < len) {
        // enqueue
        size_t remain = len - total_sent;
        outgoing_.write(data + total_sent, remain);
    }
}

void Connection::queue_and_send(const char *data, size_t len) {
    outgoing_.write(data, len);
    onPollOut();
}

shitty::StreamBuf& Connection::outgoingStreamBuf() {
    return outgoing_;
}
