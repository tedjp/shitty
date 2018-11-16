#pragma once

#include <functional>

#include "Transport.h"

namespace shitty::http1 {

class ClientTransport:
    virtual public shitty::ClientTransport,
    virtual public shitty::http1::Transport {
public:
    using resp_handler_t = std::function<void(Response&&)>;

    ClientTransport(Connection *connection, resp_handler_t&& handler);

    // from shitty::ClientTransport
    void sendRequest(const Request& request) override;

    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

protected:
    void handle(Response&&);

    Connection *connection_;
    resp_handler_t handler_ = nullptr;
};

}
