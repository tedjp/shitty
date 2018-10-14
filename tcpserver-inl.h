#include <memory>
#include <netdb.h>

#include "error.h"
#include "socket.h"

namespace shitty {

template <typename HandlerType>
const std::string TCPServer<HandlerType>::DEFAULT_HOST("::");
template <typename HandlerType>
const std::string TCPServer<HandlerType>::DEFAULT_SERVICE("http");

template <typename HandlerType>
TCPServer<HandlerType>::TCPServer(Socket&& socket):
    socket_(std::move(socket))
{}

template <typename HandlerType>
TCPServer<HandlerType>
TCPServer<HandlerType>::Create(const std::string& host, const std::string& port) {
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

    return TCPServer<HandlerType>(std::move(socket));
}

template <typename HandlerType>
void
TCPServer<HandlerType>::run() {
    // TODO
}

} // namespace shitty
