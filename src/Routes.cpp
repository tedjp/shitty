#include "Routes.h"

#include "Responder.h"

using namespace shitty;

namespace {
    void defaultHandler(Request&&, Responder&& responder) {
        static const Response response(404, "No handler for this path.\n");
        responder.respond(response);
    }
}

void Routes::addHandler(std::string_view path, RequestHandler&& handler) {
    handlers_.emplace_back(path, std::move(handler));
}

void Routes::dispatch(Request&& request, Responder&& responder) {
    for (const auto& [path, handler] : handlers_) {
        if (request.path().starts_with(path)) {
            handler(std::move(request), std::move(responder));
            return;
        }
    }

    // Default handler
}
