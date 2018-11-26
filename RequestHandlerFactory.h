#pragma once

#include <memory>
#include <tuple>

#include "RequestHandler.h"

namespace shitty {

class RequestHandlerFactory {
public:
    virtual std::unique_ptr<RequestHandler> getHandler() const = 0;
};

}
