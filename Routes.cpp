#include "Routes.h"

using namespace shitty;

std::unique_ptr<RequestHandler>
Routes::getHandler(const Request& request) const
{
    for (const auto& route: routes_) {
        const auto& path = route->path();
        if (request.path().compare(0, path.size(), path) == 0) {
            return route->getHandler();
        }
    }

    return {};
}
