#pragma once

#include <vector>

#include "Request.h"
#include "RequestHandler.h"
#include "Route.h"
#include "ServerTransport.h"

namespace shitty {

class RequestRouter {
public:
    RequestRouter() = default;
    RequestRouter(RequestRouter&&) = default;
    RequestRouter(const RequestRouter&) = default;
    RequestRouter& operator=(RequestRouter&&) = default;
    RequestRouter& operator=(const RequestRouter&) = default;
    ~RequestRouter() = default;

    RequestRouter(const std::vector<Route>* routes);

    std::unique_ptr<RequestHandler> getHandler(const Request& request);

private:
    const std::vector<Route>* routes_;
};

inline RequestRouter::RequestRouter(const std::vector<Route>* routes):
    routes_(routes)
{}

}
