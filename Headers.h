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
    void set(const std::string &name, const std::string& value);
    void add(const std::string &name, const std::string& value);

//private:
    std::unordered_multimap<std::string, std::string> kv_;
};

} // namespace shitty
