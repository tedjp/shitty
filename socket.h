#pragma once

#include "safefd.h"

namespace shitty {

class Socket {
public:
    explicit Socket(int domain, int type, int protocol);
    explicit Socket(SafeFD&& fd);

    void bind(const struct sockaddr *addr, socklen_t len);
    void listen(int backlog = 1);
    Socket accept(
            struct sockaddr_storage *addr = nullptr,
            socklen_t *addrlen = nullptr,
            int flags = 0);

    ssize_t read(void *buf, size_t len);
    ssize_t write(const void *buf, size_t len);

    //IOBuf read(size_t limit = SIZE_MAX);

private:
    SafeFD fd_;
};

} // namespace shitty
