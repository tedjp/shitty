#include "ServerStream.h"
#include "StaticResponder.h"

using shitty::StaticResponder;

void StaticResponder::onRequest(Request&& req, ServerStream *stream) {
    stream->sendResponse(response_);
}

void StaticResponder::addStandardHeaders() {
    response_.message.headers().set(
            "content-length",
            std::to_string(response_.message.body().size()));
}
