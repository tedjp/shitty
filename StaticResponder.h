#pragma once

#include "Handler.h"
#include "Response.h"

namespace shitty {

class StaticResponder: public Handler {
public:
    // Allow construction using all Response c'tors
    template <typename... Args>
    StaticResponder(Args... args):
        response_(args...)
    {}

    void handle(Request&& req, Transport *transport) override;

private:
    Response response_;
};

}
