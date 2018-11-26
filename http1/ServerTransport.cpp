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
    // FIXME: Could be nullptr!
    request_handler_->onRequest(std::move(req), this);
}
