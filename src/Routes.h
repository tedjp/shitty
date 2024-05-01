#pragma once

#include <memory>
#include <vector>

#include "Request.h"
#include "RequestHandler.h"
#include "Route.h"

namespace shitty {

class Routes {
public:
    Routes() = default;

    void addRoute(std::unique_ptr<Route>&& route) {
        routes_.emplace_back(std::move(route));
    }

    std::unique_ptr<RequestHandler>
    getHandler(const Request& request) const;

private:
    std::vector<std::unique_ptr<Route>> routes_;
};

}
