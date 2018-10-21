#include <memory>
#include <netdb.h>
#include <poll.h>

#include "error.h"
#include "socket.h"
#include "tcpserver.h"

namespace shitty::tcpserver {

const std::string Server::DEFAULT_HOST("::");
const std::string Server::DEFAULT_SERVICE("http");

Server::Server(uint16_t port):
    Server(Server::DEFAULT_HOST, std::to_string(port))
{}

Server::Server(const std::string& host, const std::string& service):
    socket_(SafeFD(-1))
{
    const struct addrinfo hints{
        .ai_flags = AI_PASSIVE | AI_V4MAPPED,
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_addrlen = 0,
        .ai_addr = nullptr,
        .ai_canonname = nullptr,
        .ai_next = nullptr,
    };

    struct freeaddrinfo_t{
        void operator()(struct addrinfo *addrs) {
            ::freeaddrinfo(addrs);
        }
    };

    std::unique_ptr<struct addrinfo, freeaddrinfo_t> addrs;
    struct addrinfo *addrs_raw = nullptr;
    int err = getaddrinfo(
            host.empty() ? nullptr : host.c_str(),
            service.empty() ? nullptr : service.c_str(),
            &hints,
            &addrs_raw);

    if (err != 0)
        throw error_errno("getaddrinfo");

    if (addrs_raw == nullptr)
        throw std::runtime_error("No addresses");

    addrs.reset(addrs_raw);
    addrs_raw = nullptr;

    // FIXME: Try all addresses
    socket_ = Socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    socket_.bind(addrs->ai_addr, addrs->ai_addrlen);
    socket_.listen();
}

void
Server::run() {
    struct pollfd pfd = {
        .fd = socket_.getRawFD(),
        .events = POLLIN,
        .revents = 0,
    };

    for (;;) {
        int e = poll(&pfd, 1, -1);
        if (e == -1)
            throw error_errno("poll");
        onNewConnection(socket_.accept());
    }
}

int
Server::getFD() {
    return socket_.getRawFD();
}

void Server::onEvent() {
    onNewConnection(socket_.accept());
}

} // namespace shitty
