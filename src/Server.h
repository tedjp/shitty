#pragma once

#include <memory>
#include <vector>

#include "Routes.h"

namespace shitty {

class Server {
public:
    Server();
    ~Server();

    Server& addStaticHandler(const std::string& path, Response&& response) {
        routes_.addRoute(std::make_unique<StaticRoute>(path, std::make_unique<StaticResponder>(std::move(response))));
        return *this;
    }

    Server& addStaticHandler(const std::string& path, std::unique_ptr<StaticResponder>&& responder) {
        routes_.addRoute(std::make_unique<StaticRoute>(path, move(responder)));
        return *this;
    }

    template <typename... Args>
    Server& addStaticHandler(const std::string& path, Args... args) {
        routes_.addRoute(std::make_unique<StaticRoute>(path, std::make_unique<StaticResponder>(std::forward<Args>(args)...)));
        return *this;
    }

    template <typename... Args>
    Server& addHandler(const std::string& path, Args... args) {
        routes_.addRoute(std::make_unique<FactoryRoute>(path, std::forward<Args>(args)...));
        return *this;
    }

    void addRoute(std::unique_ptr<Route>&& route) {
        routes_.addRoute(std::move(route));
    }

    void run();

    int epollFD();

private:
    // Config
    // TODO: Move all config into a separate class.
    Routes routes_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shitty
