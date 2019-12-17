#pragma once

#include <unordered_map>
#include "StringUtils.h"

namespace shitty {

using Header = std::pair<const std::string, std::string>;

extern const Header no_header;

class Headers {
public:
    Headers() = default;
    Headers(std::initializer_list<Header> headers):
        kv_(headers)
    {}
    Headers(std::initializer_list<std::string> headers);
    void set(const std::string &name, const std::string& value);
    void add(const std::string &name, const std::string& value);

    const Header& get(const std::string& name) const;

//private:
    std::unordered_multimap<
        std::string,
        std::string,
        ascii::CaseInsensitiveHash,
        ascii::CaseInsensitiveEqual
    > kv_;
};

// Set Date header and set Server header if not already set.
void setStandardHeaders(Headers& headers);

void setContentLength(Headers& headers, size_t content_length);

} // namespace shitty
