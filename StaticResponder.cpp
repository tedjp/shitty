#include "StaticResponder.h"

using shitty::StaticResponder;

void StaticResponder::handle(Request&& req, ServerTransport *transport) {
    transport->writeResponse(response_);
}

void StaticResponder::addStandardHeaders() {
    response_.message.headers().set(
            "content-length",
            std::to_string(response_.message.body().size()));
}
