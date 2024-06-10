#pragma once

#include <memory>

#include "Request.h"
#include "Response.h"
#include "Stream.h"

namespace shitty {

class Request;
class Response;
class Server;

class ServerStream: public Stream {
public:
    virtual void sendResponse(const Response&) = 0;
    virtual void onRequest(Request&&) = 0;

private:
    // Never nullptr
    Server* server_ = nullptr;
};

} // namespace shitty
