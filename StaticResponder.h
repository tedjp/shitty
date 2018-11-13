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
    {
        addStandardHeaders();
    }

    void handle(Request&& req, Transport *transport) override;

protected:
    void addStandardHeaders();

private:
    Response response_;
};

}
