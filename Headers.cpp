#include <ctime>

#include "Error.h"
#include "Headers.h"
#include "HTTPDate.h"
#include "StringUtils.h"

using std::string;
using shitty::Headers;

Headers::Headers(std::initializer_list<std::string> headers) {
    for (auto& hstr: headers) {
        auto col = hstr.find(':');
        if (col == hstr.npos)
            throw std::runtime_error("Malformed string header; ':' required");

        std::string value = hstr.substr(col);
        std::string name = std::move(hstr);
        name.resize(col);

        trimLWS(name);
        asciiLower(name);
        trimLWS(value);

        kv_.emplace(std::move(name), std::move(value));
    }
}

void Headers::set(const string& name, const string& value) {
    // TODO: case-fold here.
    kv_.erase(name);
    kv_.emplace(name, value);
}

void Headers::add(const string& name, const string& value) {
    // TODO: case-fold here.
    kv_.emplace(name, value);
}

// These functions access the kv_ property directly because it avoids
// case-folding that will at some point be implemented in the set() and add()
// functions.

void shitty::setContentLength(Headers& headers, size_t content_length) {
    if (headers.kv_.find("content-length") != headers.kv_.end())
        return; // Trust the caller, it's faster ;)

    headers.kv_.emplace("content-length", std::to_string(content_length));
}

static void setDateHeader(Headers& headers) {
    // Ideally the client sets the Date header immediately *before* generating
    // the response (RFC 7231 § 7.1.1.2 "Date" para. 3), so it might already be
    // set to some time in the recent past.
    if (headers.kv_.find("date") != headers.kv_.end())
        return;

    headers.kv_.emplace("date", shitty::HTTPDate::now());
}

static void setServerHeader(Headers& headers) {
    if (headers.kv_.find("server") != headers.kv_.end())
        return;

    headers.kv_.emplace("server", "Shitty");
}

void shitty::setStandardHeaders(Headers& headers) {
    setDateHeader(headers);
    setServerHeader(headers);
}
