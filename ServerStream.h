#pragma once

#include <memory>

#include "Stream.h"
#include "RequestHandler.h"

namespace shitty {

class Request;
class Response;

class ServerStream: public Stream {
public:
    virtual void sendResponse(const Response&) = 0;
    virtual void onRequest(Request&&) = 0;

private:
    std::unique_ptr<RequestHandler> handler_;
};

}
