#pragma once

#include <sys/socket.h>

#include "safefd.h"

namespace shitty {

class Socket {
public:
    explicit Socket(int domain, int type, int protocol);
    explicit Socket(SafeFD&& fd);

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&&) = default;
    Socket& operator=(Socket&&) = default;

    void bind(const struct sockaddr *addr, socklen_t len);
    void listen(int backlog = 1);
    Socket accept(
            struct sockaddr_storage *addr = nullptr,
            socklen_t *addrlen = nullptr,
            int flags = 0);

    ssize_t read(void *buf, size_t len);
    ssize_t write(const void *buf, size_t len);

    // XXX: Probably better to make this inherit from Pollable
    // with a friend class that can access the FD?
    int getRawFD() { return fd_.get(); }

    void close() {
        fd_.close();
    }

    explicit operator bool() const {
        return fd_;
    }

    void send(const void *buf, size_t len);

    //IOBuf read(size_t limit = SIZE_MAX);

private:
    SafeFD fd_;
};

} // namespace shitty
