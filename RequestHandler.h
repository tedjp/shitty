#pragma once

#include "Request.h"

namespace shitty {

class ServerTransport;

class RequestHandler {
public:
    RequestHandler() = default;
    RequestHandler(RequestHandler&&) = default;
    RequestHandler(const RequestHandler&) = default;
    RequestHandler& operator=(const RequestHandler&) = default;
    RequestHandler& operator=(RequestHandler&&) = default;
    virtual ~RequestHandler();

    virtual void onRequest(Request&& request, ServerTransport *transport) = 0;
};

} // namespace
