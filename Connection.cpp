#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Connection.h"
#include "HTTP1Transport.h"

using shitty::Connection;

Connection::Connection(int epfd, int fd, RequestRouter* request_router):
    fd_(fd),
    epfd_(epfd),
    request_router_(request_router),
    transport_(std::make_unique<HTTP1Transport>(this, request_router_))
{
    if (epfd_ < 0)
        throw std::invalid_argument("Bad event FD");

    if (fd_ < 0)
        throw std::invalid_argument("Connection fd invalid");
}

Connection::Connection(Connection&& other) noexcept:
    fd_(-1),
    epfd_(-1),
    incoming_(std::move(other.incoming_)),
    outgoing_(std::move(other.outgoing_)),
    request_router_(std::move(other.request_router_)),
    transport_(std::move(other.transport_))
{
    std::swap(epfd_, other.epfd_);
    std::swap(fd_, other.fd_);
}

Connection& Connection::operator=(Connection&& other) noexcept {
    std::swap(epfd_, other.epfd_);
    std::swap(fd_, other.fd_);
    std::swap(incoming_, other.incoming_);
    std::swap(outgoing_, other.outgoing_);
    std::swap(request_router_, other.request_router_);
    std::swap(transport_, other.transport_);
    return *this;
}

Connection::~Connection() {
    try {
        close();
    } catch (...)
    {}
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
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd_, nullptr);

    epfd_ = -1;
    ::close(fd_);

    // TODO: Notify Server that this object should be destroyed
    // (but don't self-destruct because there could be more epoll events with
    // this object's pointer).
}

void Connection::onPollOut() {
    // TODO: flush pending writes
}

void Connection::send(const void *data, size_t len) {
    // FIXME: Queue unsent outgoing data
    ::send(fd_, data, len, 0);
}
