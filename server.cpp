#include <memory>
#include <netdb.h>

#include "error.h"
#include "server.h"

namespace shitty::http2 {

const std::string Server::DEFAULT_HOST("::");
const std::string Server::DEFAULT_SERVICE("http");

Server::Server(Socket&& socket):
    socket_(std::move(socket))
{}

Server
Server::Create(const std::string& host, const std::string& port) {
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

    return Server(std::move(socket));
}

} // namespace shitty::http2
