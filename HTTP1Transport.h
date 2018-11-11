#pragma once

#include <utility>

#include "Connection.h"
#include "Headers.h"
#include "Request.h"
#include "RequestRouter.h"
#include "StreamBuf.h"

namespace shitty {

class HTTP1Transport: public Transport {
public:
    HTTP1Transport(Connection *connection, RequestRouter *request_router):
        connection_(connection),
        request_router_(request_router)
    {}

    void onInput(StreamBuf& input_buffer) override;

    void writeResponse(const Response& response) override;

private:
    static const char*
        findEndOfLine(const char* buffer, size_t len);
    static std::string
        renderHeaders(const Headers& headers);
    static std::string
        statusLine(const Response& request);
    static std::string
        requestLine(const Request& request);
    static std::optional<std::string>
        getLine(StreamBuf& buf);
    static Request
        requestFromRequestLine(const std::string& request_line);

    // TODO: Move all this request processing into an IncomingRequest class that
    // is fed a buffer.
    // TODO: Stop doing std::string ops on headers; just clone the entire header
    // blob and use std::string_view to point into it. Only headers with
    // continuation lines would need dynamic allocations. Then create a superset
    // string_or_view class that seamlessly owns & wraps either one.
    bool isRequestComplete();
    void onEndOfRequestHeaders();
    void onEndOfRequest();
    void headerContinuation(std::string&& line);
    void headerLine(std::string&& line);
    void readBody(StreamBuf& input);
    void readChunked(StreamBuf& input);
    void readContent(StreamBuf& input);
    void handleRequest();
    void resetIncomingRequest();
    bool processInput(StreamBuf& input);

    std::optional<Request> request_in_progress_;
    std::string last_header_; // used for header line continuations. empty == none
    bool request_headers_complete_ = false;
    // 0: no body. -1: chunked: >0: Content-Length
    // Chunked reader should simply reset it to the actual body length when the
    // terminal chunk is read.
    ssize_t expected_body_length_ = 0;

    Connection *connection_;
    RequestRouter *request_router_;
};

}
