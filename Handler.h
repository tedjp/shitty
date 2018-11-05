#pragma once

#include "Request.h"
#include "Transport.h"

namespace shitty {

class Handler {
public:
    virtual void handle(Request&& request, Transport *transport) = 0;
};

} // namespace
