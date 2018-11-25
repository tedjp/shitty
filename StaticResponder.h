#pragma once

#include "RequestHandler.h"
#include "Response.h"

namespace shitty {

class StaticResponder: public RequestHandler {
public:
    // Allow construction using all Response c'tors
    template <typename... Args>
    StaticResponder(Args... args):
        response_(args...)
    {
        addStandardHeaders();
    }

    void onRequest(Request&& req, ServerTransport *transport) override;

protected:
    void addStandardHeaders();

private:
    Response response_;
};

}
