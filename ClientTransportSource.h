#pragma once

#include <memory>
#include <stack>
#include <unordered_map>

#include "ClientTransport.h"
#include "Connection.h"

namespace shitty {

struct SchemeAndAuthority {
    std::string scheme, authority;
};

class ClientTransportSource {
public:
    ClientTransportSource(int epfd): epfd_(epfd) {}

    ClientTransport*
    getTransport(ClientTransport::resp_handler_t&& resp_handler);

    void
    putTransport(ClientTransport*);

private:
    inline bool hasSpareTransports();

    ClientTransport* newTransport();
    ClientTransport* spareTransport();

    int epfd_ = -1;

    std::stack<std::unique_ptr<Connection>> spare_connections_;
    std::unordered_map<ClientTransport*, std::unique_ptr<Connection>> in_use_connections_;
};

inline bool ClientTransportSource::hasSpareTransports() {
    return !spare_connections_.empty();
}

}
