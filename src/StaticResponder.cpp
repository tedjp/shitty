#include "StaticResponder.h"

#include "Responder.h"

using namespace shitty;

void StaticResponder::operator()(Request&& request, Responder&& responder) {
    responder.respond(response_);
}

void StaticResponder::addStandardHeaders() {
    response_.message.headers().set(
            "content-length",
            std::to_string(response_.message.body().size()));
}
