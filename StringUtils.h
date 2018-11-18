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

}

#include "StringUtils-inl.h"
