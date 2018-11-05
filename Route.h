#pragma once

#include <memory>
#include <string>

#include "Handler.h"

namespace shitty { 

struct Route {
    template <typename HandlerT>
    Route(const std::string& path_param, HandlerT&& handler_param):
        path(path_param),
        handler(std::make_unique<HandlerT>(std::move(handler_param)))
    {}

    std::string path;
    std::unique_ptr<Handler> handler;
};

} // namespace shitty
