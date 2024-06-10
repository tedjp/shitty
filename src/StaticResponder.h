#pragma once

#include "Response.h"

namespace shitty {

class Request;
class Responder;

// Responds to a request with the same response every time.
class StaticResponder {
public:
    StaticResponder(Response response):
        response_(std::move(response))
    {
        addStandardHeaders();
    }

    void operator()(Request&& req, Responder&& responder);

protected:
    void addStandardHeaders();

private:
    Response response_;
};

}
