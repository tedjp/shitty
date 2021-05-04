#include "Date.h"
#include "Stream.h"

using namespace shitty;

static const std::string PRODUCT("Shitty");

static void setDateHeader(Headers& headers) {
    // Ideally the client sets the Date header immediately *before* generating
    // the response (RFC 7231 § 7.1.1.2 "Date" para. 3), so it might already be
    // set to some time in the recent past.
    if (headers.kv_.find("date") != headers.kv_.end())
        return;

    headers.kv_.emplace("date", Date::now());
}

static void setServerHeader(Headers& headers) {
    if (headers.kv_.find("server") != headers.kv_.end())
        return;

    headers.kv_.emplace("server", PRODUCT);
}

void Stream::setGeneralServerHeaders(Headers& headers) {
    setDateHeader(headers);
    setServerHeader(headers);
}

void Stream::setUserAgentHeader(Headers& headers) {
    if (headers.kv_.find("user-agent") != headers.kv_.end())
        return;

    headers.kv_.emplace("user-agent", PRODUCT);
}
