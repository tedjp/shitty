#pragma once

#include <optional>
#include <string>

#include "../Headers.h"
#include "../Request.h"
#include "../Response.h"
#include "../StreamBuf.h"

namespace shitty::http1 {

struct RequestLine {
    std::string method, path, version;
};

struct StatusLine {
    uint_fast16_t code;
    std::string reason_phrase, version;
};

std::string
renderRequest(const Request&);

std::string
renderHeaders(const Headers& headers);

ssize_t
getContentLength(const Headers& headers);

std::string
statusLine(uint_fast16_t statusCode);

std::string
statusLine(const Response& response);

std::string
requestLine(const Request& request);

RequestLine
parseRequestLine(const std::string& request_line);

StatusLine
parseStatusLine(const std::string& status_line);

std::optional<std::string>
getLine(StreamBuf& buf);

}
