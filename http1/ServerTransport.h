#pragma once

#include "HTTP1.h"
#include "Transport.h"
#include "../Routes.h"
#include "../ServerStream.h"

namespace shitty::http1 {

class ServerTransport:
    public shitty::ServerStream,
    public shitty::http1::Transport {
public:
    ServerTransport(
            Connection* connection,
            const Routes* routes);

    ServerTransport(Connection* connection);

    // from shitty::ServerStream
    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;

protected:
    // from shitty::http1::Transport
    void handleIncomingMessage(IncomingMessage&&) override;
    void onEndOfMessageHeaders(Headers& headers) override;

private:
    bool tryUpgrade(Request& request);
    void confirmUpgrade(const std::string& token);
    void upgrade(
            const std::string& token,
            std::unique_ptr<shitty::Transport>&& newTransport,
            Request&& request);

    virtual void handleExpect(const std::string& value);

    const Routes* routes_ = nullptr;
    std::unique_ptr<RequestHandler> request_handler_;
};

} // namespace
