#pragma once

#include <memory>
#include <vector>

#include "Routes.h"

namespace shitty {

class Response;

class Server {
public:
    Server();
    ~Server();

    // Add a route with a static response.
    Server& addRoute(std::string_view path, Response response);

    // Add a route with a request handler.
    Server& addRoute(std::string_view path, RequestHandler handler);

    void run();

    int epollFD();

    // Called by ServerStreams to dispatch a request to a request handler.
    void dispatch(Request&& request, Responder&& responder);

private:
    // Config
    // TODO: Move all config into a separate class.
    Routes routes_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shitty
