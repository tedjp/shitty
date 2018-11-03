#pragma once

#include <ostream>
#include <string>

#include "responder.h"
#include "static_response.h"

namespace shitty {

class StaticResponder: public Responder {
public:
    template <typename... Args>
    StaticResponder(Args... args):
        response_(args...)
    {
    }

    void writeHeaders(std::ostream& dest) override;
    void writeBody(std::ostream& dest) override;

private:
    StaticResponse response_;
};

}
