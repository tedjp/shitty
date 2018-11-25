#include "HTTP1.h"
#include "ServerTransport.h"

using namespace shitty::http1;
using shitty::Response;

ServerTransport::ServerTransport(
        Connection* connection,
        RequestRouter* request_router):
    shitty::http1::Transport(connection),
    request_router_(std::move(request_router))
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
    request_handler_ = request_router_->getHandler(req);
    // FIXME: Could be nullptr!
    request_handler_->onRequest(std::move(req), this);
}
