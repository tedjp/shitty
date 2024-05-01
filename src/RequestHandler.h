#pragma once

#include "Request.h"

namespace shitty {

class ServerStream;

class RequestHandler {
public:
    RequestHandler() = default;
    RequestHandler(RequestHandler&&) = default;
    RequestHandler(const RequestHandler&) = default;
    RequestHandler& operator=(const RequestHandler&) = default;
    RequestHandler& operator=(RequestHandler&&) = default;
    virtual ~RequestHandler();

    virtual void onRequest(Request&& request, ServerStream *stream) = 0;
};

} // namespace
