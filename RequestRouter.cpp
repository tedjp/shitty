#include "RequestRouter.h"
#include "Response.h"
#include "UnhandledRequestHandler.h"

using namespace shitty;

std::unique_ptr<RequestHandler>
RequestRouter::getHandler(const Request& request) {
    for (const auto& route: *routes_) {
        if (request.path().compare(0, route.path.size(), route.path) == 0) {
            return route.handler_factory->getHandler();
        }
    }

    return {};
}
