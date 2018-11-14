#pragma once

#include <unordered_map>

namespace shitty {

using Header = std::pair<const std::string, std::string>;

class Headers {
public:
    Headers() = default;
    Headers(std::initializer_list<Header> headers):
        kv_(headers)
    {}
    Headers(std::initializer_list<std::string> headers);
    void set(const std::string &name, const std::string& value);
    void add(const std::string &name, const std::string& value);

//private:
    std::unordered_multimap<std::string, std::string> kv_;
};

// Set Date header and set Server header if not already set.
void setStandardHeaders(Headers& headers);

void setContentLength(Headers& headers, size_t content_length);

} // namespace shitty
