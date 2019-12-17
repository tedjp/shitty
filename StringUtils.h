#pragma once

#include <string>
#include <utility>

namespace shitty {

inline void trimLWS(std::string& s);
inline void trimLeadingLWS(std::string& s);
inline void trimTrailingLWS(std::string& s);
inline void asciiLower(std::string& s);

inline std::pair<std::string, std::string>
split(std::string&& src, std::string::size_type pos);

namespace ascii {

struct CaseInsensitiveHash {
    inline size_t operator()(const std::string& s) const;
};
struct CaseInsensitiveEqual {
    inline bool operator()(const std::string& left, const std::string& right) const;
};

} // namespace ascii
} // namespace shitty

#include "StringUtils-inl.h"
