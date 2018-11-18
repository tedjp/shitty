#include <algorithm>
#include <cctype>
#include <cstring>

#include "StringUtils.h"

// These are all good candidates to be declared inline.

void shitty::trimTrailingLWS(std::string& s) {
    size_t initial_size = s.size();
    size_t i = initial_size;
    while (i != 0) {
        --i;

        if (s[i] == ' ' || s[i] == '\t')
            continue;

        break;
    }

    size_t new_size = i + 1;
    if (new_size != initial_size)
        s.resize(i);
}

static inline bool isLWS(char c) {
    return c == ' ' || c == '\t';
}

void shitty::trimLeadingLWS(std::string& s) {
    size_t leading_whitespace = 0;
    size_t slen = s.size();
    for (size_t i = 0; i < slen && isLWS(s[i]); ++i)
        ++leading_whitespace;

    if (leading_whitespace == 0)
        return;

    size_t new_size = slen - leading_whitespace;
    memmove(s.data(), s.data() + leading_whitespace, new_size);
    s.resize(new_size);
}

void shitty::trimLWS(std::string& s) {
    trimTrailingLWS(s);
    trimLeadingLWS(s);
}

void shitty::asciiLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](char c) {
            return static_cast<char>(::tolower(c));
        });
}

// A moving-split operation that only allocates one new string, reusing the
// input for one of the output parameters.
// Convenient in combination with C++17 structured bindings:
// auto [left, right] = split(std::move(my_string), separator);
std::pair<std::string, std::string>
shitty::split(std::string&& src, std::string::size_type pos) {
    std::string second = src.substr(pos);
    src.resize(pos);
    return {std::move(src), std::move(second)};
}
