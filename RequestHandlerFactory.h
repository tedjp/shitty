#pragma once

#include <memory>
#include <tuple>

#include "RequestHandler.h"

namespace shitty {

class RequestHandlerFactory {
public:
    virtual std::unique_ptr<RequestHandler> getHandler() const = 0;
};

template <typename HandlerT, typename... HandlerArgs>
class AutoRequestHandlerFactory: public RequestHandlerFactory {
public:
    AutoRequestHandlerFactory(HandlerArgs... args):
        handler_args_(std::move(args)...)
    {}

    std::unique_ptr<RequestHandler> getHandler() const override {
        return std::make_unique<HandlerT>(std::make_from_tuple<HandlerT>(handler_args_));
    }

private:
    std::tuple<HandlerArgs...> handler_args_;
};

}
