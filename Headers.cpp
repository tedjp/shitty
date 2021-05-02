#include <ctime>

#include "Error.h"
#include "Headers.h"
#include "Date.h"
#include "StringUtils.h"

using std::string;

namespace shitty {

const Header no_header;

Headers::Headers(std::initializer_list<std::string> headers) {
    for (auto& hstr: headers) {
        auto col = hstr.find(':');
        if (col == hstr.npos)
            throw std::runtime_error("Malformed string header; ':' required");

        std::string value = hstr.substr(col + 1);
        std::string name = std::move(hstr);
        name.resize(col);

        trimLWS(name);
        trimLWS(value);

        kv_.emplace(std::move(name), std::move(value));
    }
}

void Headers::set(const string& name, const string& value) {
    kv_.erase(name);
    kv_.emplace(name, value);
}

void Headers::add(const string& name, const string& value) {
    kv_.emplace(name, value);
}

void Headers::remove(const string& name) {
    kv_.erase(name);
}

const Header& Headers::get(const std::string& name) const {
    auto it = kv_.find(name);
    if (it != kv_.end())
        return *it;

    return no_header;
}

// These functions access the kv_ property directly because it avoids
// case-folding that will at some point be implemented in the set() and add()
// functions.

void setContentLength(Headers& headers, size_t content_length) {
    if (headers.kv_.find("content-length") != headers.kv_.end())
        return; // Trust the caller, it's faster ;)

    headers.kv_.emplace("content-length", std::to_string(content_length));
}

} // namespace shitty
