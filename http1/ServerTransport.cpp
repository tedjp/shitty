#include "HTTP1.h"
#include "ServerTransport.h"

using namespace shitty::http1;
using shitty::Response;

ServerTransport::ServerTransport(
        Connection* connection,
        req_handler_t&& request_handler):
    shitty::http1::Transport(connection),
    request_handler_(request_handler)
{
}

void ServerTransport::sendResponse(const Response& resp) {
    sendMessage(statusLine(resp), resp.message);
}

void ServerTransport::handleIncomingMessage(IncomingMessage&& msg) {
    auto request_line = parseRequestLine(std::move(msg.first_line));
    handle(Request(request_line.method, request_line.path, std::move(msg.message)));
}

void ServerTransport::handle(Request&& req) {
    request_handler_(std::move(req), this);
}
