#pragma once

#include <vector>

#include "Request.h"
#include "Route.h"

namespace shitty {

class RequestRouter {
public:
    RequestRouter() = default;
    RequestRouter(const std::vector<Route>* handlers);

    // XXX: std::unique_ptr<Request> instead?
    void route(Request&& request, Transport *transport);

private:
    const std::vector<Route>* handlers_;
};

inline RequestRouter::RequestRouter(const std::vector<Route>* handlers):
    handlers_(handlers)
{}

}
