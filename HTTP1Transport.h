#pragma once

#include "Connection.h"
#include "Headers.h"
#include "Request.h"
#include "RequestRouter.h"
#include "StreamBuf.h"

namespace shitty {

class HTTP1Transport: public Transport {
public:
    HTTP1Transport(Connection *connection, RequestRouter *router):
        connection_(connection),
        request_router_(router)
    {}

    void onInput(StreamBuf& input_buffer) override;

    void writeResponse(const Response& response) override;

private:
    static char*
        findEndOfHeader(char* buffer_, size_t len);
    static Headers
        parseHeaders(const char *buffer, size_t len);
    static std::string
        renderHeaders(const Headers& headers);
    static std::string
        statusLine(const Response& request);
    static std::string
        requestLine(const Request& request);

    //std::optional<Request> request_in_progress;

    Connection *connection_;
    RequestRouter *request_router_;
};

}
