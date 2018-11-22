#pragma once

#include <functional>

#include "Transport.h"

namespace shitty::http1 {

class ClientTransport:
    virtual public shitty::ClientTransport,
    virtual public shitty::http1::Transport {
public:
    ClientTransport(Connection* connection, resp_handler_t&& handler = resp_handler_t());

    // from shitty::ClientTransport
    void sendRequest(const Request& request) override;
    void setHandler(resp_handler_t&& handler) override;
    void resetHandler() override;

    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

protected:
    void handle(Response&&);

    Connection* connection_;
    resp_handler_t handler_ = nullptr;
};

}
