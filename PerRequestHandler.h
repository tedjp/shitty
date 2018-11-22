#pragma once

#include "Handler.h"

namespace shitty {

template <typename HandlerT>
class PerRequestHandler: public Handler {
    void handle(Request&& request, ServerTransport *transport) override {
        HandlerT instance;

        instance.handle(std::move(request), transport);
    }
};

}
