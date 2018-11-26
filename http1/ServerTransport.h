#pragma once

#include "Transport.h"
#include "../Routes.h"
#include "../ServerTransport.h"

namespace shitty::http1 {

class ServerTransport:
    virtual public shitty::ServerTransport,
    virtual public shitty::http1::Transport {
public:
    ServerTransport(
            Connection* connection,
            Routes* routes);

    // from shitty::ServerTransport
    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;

protected:
    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;

private:
    Routes* routes_;
    std::unique_ptr<RequestHandler> request_handler_;
};

} // namespace
