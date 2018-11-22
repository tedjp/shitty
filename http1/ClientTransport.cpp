#include "ClientTransport.h"
#include "HTTP1.h"

using namespace shitty::http1;
using shitty::Request;
using shitty::Response;

ClientTransport::ClientTransport(
        std::unique_ptr<Connection>&& connection,
        resp_handler_t&& handler):
    shitty::http1::Transport(connection.get()),
    connection_(std::move(connection)),
    handler_(std::move(handler))
{
    // TODO: Relax this requirement so that it's only required to be non-null
    // when sendRequest() is called.
    if (handler == nullptr)
        throw std::invalid_argument("ClientTransport requires a non-null Handler");
}

void ClientTransport::sendRequest(const Request& req) {
    sendMessage(requestLine(req), req.message_);
}

void ClientTransport::handleIncomingMessage(IncomingMessage&& msg) {
    auto status_line = parseStatusLine(std::move(msg.first_line));
    handle(Response(status_line.code, std::move(msg.message)));
}

void ClientTransport::handle(Response&& resp) {
    handler_(std::move(resp), this);
}

void ClientTransport::setHandler(resp_handler_t&& handler) {
    handler_ = std::move(handler);
}

void ClientTransport::resetHandler() {
    handler_ = nullptr;
}
