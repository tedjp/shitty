#pragma once

#include <memory>
#include <vector>

#include "Routes.h"

namespace shitty {

class Server {
public:
    Server();
    ~Server();

    template <typename... Args>
    Server& addStaticHandler(const std::string& path, Args... args) {
        routes_.addRoute(std::make_unique<StaticRoute>(path, StaticResponder(std::move(args)...)));
        return *this;
    }

    template <typename... Args>
    Server& addHandler(const std::string& path, Args... args) {
        routes_.addRoute(std::make_unique<FactoryRoute>(path, std::move(args)...));
        return *this;
    }

    void addRoute(std::unique_ptr<Route>&& route) {
        routes_.addRoute(std::move(route));
    }

    void run();

    int epollFD();

private:
    Routes routes_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shitty
