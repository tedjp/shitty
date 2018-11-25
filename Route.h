#pragma once

#include <memory>
#include <string>

#include "RequestHandlerFactory.h"

namespace shitty { 

struct Route {
    Route(
            const std::string& path,
            std::unique_ptr<RequestHandlerFactory>&& factory):
        path(path),
        handler_factory(std::move(factory))
    {}

#if 0
    template <typename HandlerT, typename... HandlerArgs>
    Route(const std::string& path_param, HandlerArgs... handler_args):
        path(path_param),
        handler_factory(std::make_unique<AutoRequestHandlerFactory<HandlerT>>(std::move(handler_args)...))
    {}
#endif

    std::string path;
    std::unique_ptr<RequestHandlerFactory> handler_factory;
};

} // namespace shitty
