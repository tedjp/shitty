#pragma once

#include <memory>
#include <vector>

#include "Routes.h"
#include "ServerConfig.h"

namespace shitty {

class Response;

class Server {
public:
    Server();
    explicit Server(const ServerConfig& config);
    virtual ~Server();

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
    ServerConfig config_;

    Routes routes_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shitty
