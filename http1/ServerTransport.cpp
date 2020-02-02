#include <cassert>

#include "HTTP1.h"
#include "ServerTransport.h"

using namespace shitty::http1;
using shitty::Response;

ServerTransport::ServerTransport(
        Connection* connection,
        Routes* routes):
    shitty::http1::Transport(connection),
    routes_(std::move(routes))
{
}

void ServerTransport::sendResponse(const Response& resp) {
    sendMessage(statusLine(resp), resp.message);
}

void ServerTransport::handleIncomingMessage(IncomingMessage&& msg) {
    auto request_line = parseRequestLine(std::move(msg.first_line));
    onRequest(Request(request_line.method, request_line.path, std::move(msg.message)));
}

void ServerTransport::onRequest(Request&& req) {
    request_handler_ = routes_->getHandler(req);

    assert(request_handler_ != nullptr);

    request_handler_->onRequest(std::move(req), this);
}

void ServerTransport::onEndOfMessageHeaders(Headers& headers) {
    const Header& expectHeader = headers.get("expect");
    if (expectHeader != no_header) {
        handleExpect(expectHeader.second);
        return;
    }
}

// This SHALL be overridden by proxies, which, for HTTP/1.1 and later clients,
// are required to forward the request to the origin.
void ServerTransport::handleExpect(const std::string& value) {
    // TODO: If client is HTTP/1.0 or earlier, MUST ignore the Expect header.

    if (value != "100-continue") {
        Response response(417, "Expectation Failed.\n");
        sendMessage(statusLine(response), response.message);
        return;
    }

    sendResponse(Response(100, Message()));
}
