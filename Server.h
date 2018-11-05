#pragma once

#include <memory>
#include <vector>

#include "Route.h"

namespace shitty {

class Server {
public:
    Server();
    ~Server();

    template <typename... Args>
    Server& addHandler(std::string&& path, Args... args) {
        routes_.emplace_back(std::move(path), std::move(args)...);
        return *this;
    }

    void run();

private:
    std::vector<Route> routes_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shitty
