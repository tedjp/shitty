#pragma once

#include "RequestHandler.h"
#include "Response.h"

namespace shitty {

// Responds to a request with the same response every time.
class StaticResponder: public RequestHandler {
public:
    // Allow construction using all Response c'tors
    template <typename... Args>
    StaticResponder(Args... args):
        response_(std::move(args)...)
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
