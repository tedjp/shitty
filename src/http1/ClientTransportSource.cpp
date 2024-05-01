#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../Error.h"
#include "ClientTransport.h"
#include "ClientTransportSource.h"

using namespace shitty::http1;

ClientTransport*
ClientTransportSource::getTransport(ClientTransport::resp_handler_t&& resp_handler) {
    auto transport = hasSpareTransports() ? spareTransport() : newTransport();
    transport->setHandler(std::move(resp_handler));
    return transport;

}

ClientTransport*
ClientTransportSource::spareTransport() {
    auto conn = std::move(spare_connections_.top());
    auto transport = dynamic_cast<ClientTransport*>(conn->getTransport());

    spare_connections_.pop();
    in_use_connections_.try_emplace(transport, std::move(conn));

    return transport;
}

ClientTransport*
ClientTransportSource::newTransport() {
    // XXX: Use SafeFD
    int s = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (s == -1)
        throw error_errno("socket");

    struct sockaddr_in6 addr = {};
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(23206);
    addr.sin6_addr = in6addr_loopback;

    int err = connect(s, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    if (err == -1 && errno != EINPROGRESS) {
        int connect_errno = errno;
        close(s);
        throw error_errno("connect failed", connect_errno);
    }

    auto conn = std::make_unique<Connection>(epfd_, s);
    auto transport = std::make_unique<http1::ClientTransport>(*conn);
    ClientTransport* ct = transport.get();
    conn->setTransport(std::move(transport));

    in_use_connections_.try_emplace(ct, std::move(conn));

    return ct;
}

void
ClientTransportSource::putTransport(ClientTransport* transport) {
    transport->resetHandler();

    auto it = in_use_connections_.find(transport);
    if (it == in_use_connections_.end())
        throw std::logic_error("Tried to remove unknown transport");

    spare_connections_.push(std::move(it->second));
    in_use_connections_.erase(it);
}
