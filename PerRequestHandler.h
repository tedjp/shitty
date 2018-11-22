#pragma once

#include <tuple>

#include "Handler.h"

namespace shitty {

template <typename HandlerT, typename... HandlerArgs>
class PerRequestHandler: public Handler {
public:
    PerRequestHandler(HandlerArgs... args):
        init_args_(std::move(args)...)
    {}

    void handle(Request&& request, ServerTransport *transport) override {
        auto instance = std::make_from_tuple<HandlerT>(init_args_);

        instance.handle(std::move(request), transport);
    }

private:
    std::tuple<HandlerArgs...> init_args_;
};

}
