#include "RequestRouter.h"
#include "Response.h"
#include "UnhandledRequestHandler.h"

using namespace shitty;

void RequestRouter::route(Request&& request, ServerTransport *transport) {
    Handler *handler = &unhandled_request_handler;

    for (const auto& route: *handlers_) {
        if (request.path().compare(0, route.path.size(), route.path) == 0) {
            handler = route.handler.get();
            break;
        }
    }

    handler->handle(std::move(request), transport);
}
