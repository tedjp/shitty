#pragma once

#include <functional>

#include "Stream.h"

namespace shitty {

class Request;
class Response;

class ClientStream: public Stream {
public:
    using resp_handler_t = std::function<void(Response&&, ClientStream*)>;

    virtual void sendRequest(const Request&) = 0;
    virtual void setHandler(resp_handler_t&&) = 0;
    virtual void resetHandler() = 0;
};

}
