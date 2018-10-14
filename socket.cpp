#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>

#include "error.h"
#include "socket.h"

using namespace shitty;

Socket::Socket(int domain, int type, int protocol) {
    fd_ = SafeFD(::socket(domain, type, protocol));
    if (!fd_)
        throw error_errno("socket");
}

Socket::Socket(SafeFD&& fd):
    fd_(std::move(fd))
{}

void
Socket::bind(const struct sockaddr *addr, socklen_t len) {
    int err = ::bind(fd_.get(), addr, len);
    if (err == -1)
        throw error_errno("bind");
}

void Socket::listen(int backlog) {
    int err = ::listen(fd_.get(), backlog);
    if (err == -1)
        throw error_errno("listen");
}

Socket
Socket::accept(
        struct sockaddr_storage *addr,
        socklen_t *addrlen,
        int flags)
{
    SafeFD fd(::accept4(
                fd_.get(),
                reinterpret_cast<struct sockaddr*>(addr),
                addrlen,
                flags));

    if (!fd)
        throw error_errno("accept4");

    return Socket(std::move(fd));
}

void Socket::send(const void *buf, size_t len) {
    // FIXME if this blocks, it's bad.
    ssize_t sent = ::send(fd_, buf, len, MSG_DONTWAIT);

    if (sent < 0)
        throw error_errno("send");

    if (static_cast<size_t>(sent) < len)
        throw std::runtime_error("short write");
}
