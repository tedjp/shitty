#pragma once

#include "handler.h"

#include <vector>

namespace shitty {

class Server {
public:
    template <typename... Ts>
    Server& addHandler(const std::string& path, Ts... args) {
        //handlers_.emplace_back(path, std::forward(args)...);
        handlers_.emplace_back(path, args...);
        return *this;
    }

    void run();

private:
    std::vector<Handler> handlers_;
};

} // namespace shitty
