#pragma once

#include "Transport.h"

namespace shitty::http1 {

class ServerTransport:
    virtual public shitty::ServerTransport,
    virtual public shitty::http1::Transport {
public:
    ServerTransport(
            Connection* connection,
            RequestRouter* request_router);

    // from shitty::ServerTransport
    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;

protected:
    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

private:
    RequestRouter* request_router_;
    std::unique_ptr<RequestHandler> request_handler_;
};

} // namespace
