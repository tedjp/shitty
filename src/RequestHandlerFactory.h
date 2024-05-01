#pragma once

#include <memory>
#include <tuple>

#include "RequestHandler.h"

namespace shitty {

class RequestHandlerFactory {
public:
    virtual std::unique_ptr<RequestHandler> getHandler() const = 0;
};

template <typename HandlerT, typename... Args>
class SimpleRequestHandlerFactory: public RequestHandlerFactory {
public:
    SimpleRequestHandlerFactory(Args... args):
        init_args_(std::move(args)...)
    {}

    std::unique_ptr<RequestHandler> getHandler() const override {
        return std::make_unique<HandlerT>(std::make_from_tuple<HandlerT>(init_args_));
    }

private:
    std::tuple<Args...> init_args_;
};

}
