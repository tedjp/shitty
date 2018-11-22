#pragma once

#include "Transport.h"

namespace shitty::http1 {

class ServerTransport:
    virtual public shitty::ServerTransport,
    virtual public shitty::http1::Transport {
public:
    ServerTransport(
            Connection* connection,
            req_handler_t&& request_handler);

    // from shitty::ServerTransport
    void sendResponse(const Response&) override;

    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

private:
    void handle(Request&&);

    req_handler_t request_handler_;
};

} // namespace
