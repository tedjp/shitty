#pragma once

#include "Transport.h"

namespace shitty::http1 {

class ServerTransport:
    virtual public shitty::ServerTransport,
    virtual public shitty::http1::Transport {
public:
    ServerTransport(Connection *connection, RequestRouter *request_router);

    // from shitty::ServerTransport
    void sendResponse(const Response&) override;

    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

private:
    void handle(Request&&);

    RequestRouter *request_router_;
};

} // namespace
