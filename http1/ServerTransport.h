#pragma once

#include "HTTP1.h"
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
    void onEndOfMessageHeaders(Headers& headers) override;

private:
    virtual void handleExpect(const std::string& value);

    Routes* routes_;
    std::unique_ptr<RequestHandler> request_handler_;
};

} // namespace
