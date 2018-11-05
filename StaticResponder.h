#pragma once

#include "Handler.h"
#include "Response.h"

namespace shitty {

class StaticResponder: public Handler {
public:
#if 1
    // Allow construction using all Response c'tors
    template <typename... Args>
    StaticResponder(Args... args):
        //response_(std::forward(args)...)
        response_(args...)
    {}
#else
    StaticResponder(Response&& response);
#endif

    void handle(Request&& req, Transport *transport) override;

private:
    Response response_;
};

#if 0
StaticResponder::StaticResponder(Response&& response):
    response_(std::move(response))
{}
#endif

}
