#pragma once

#include <string>
#include <utility>

#include "../Headers.h"
#include "../Request.h"
#include "../Response.h"
#include "../StreamBuf.h"

namespace shitty::http1 {

std::string
renderRequest(const Request&);

std::string
renderHeaders(const Headers& headers);

ssize_t
getContentLength(const Headers& headers);

std::string
statusLine(const Response& response);

std::string
requestLine(const Request& request);

std::optional<std::string>
getLine(StreamBuf& buf);

}
