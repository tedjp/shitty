#include <memory>
#include <netdb.h>

#include "error.h"
#include "socket.h"
#include "tcpserver.h"

namespace shitty {

const std::string TCPServer::DEFAULT_HOST("::");
const std::string TCPServer::DEFAULT_SERVICE("http");

TCPServer::TCPServer(Handler& handler, Socket&& socket):
    socket_(std::move(socket)),
    handler_(handler)
{}

TCPServer
makeTCPServer(TCPServer::Handler& handler, uint16_t port) {
    return makeTCPServer(handler, TCPServer::DEFAULT_HOST, std::to_string(port));
}

TCPServer
makeTCPServer(
        TCPServer::Handler& handler,
        const std::string& host,
        const std::string& port)
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
            port.empty() ? nullptr : port.c_str(),
            &hints,
            &addrs_raw);

    if (err != 0)
        throw error_errno("getaddrinfo");

    if (addrs_raw == nullptr)
        throw std::runtime_error("No addresses");

    addrs.reset(addrs_raw);
    addrs_raw = nullptr;

    // FIXME: Try all addresses
    Socket socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    socket.bind(addrs->ai_addr, addrs->ai_addrlen);
    socket.listen();

    return TCPServer(handler, std::move(socket));
}

void
TCPServer::run() {
    // TODO
}

} // namespace shitty
