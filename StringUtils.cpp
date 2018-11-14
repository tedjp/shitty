#include <algorithm>
#include <cctype>
#include <cstring>

#include "StringUtils.h"

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

void shitty::trimLeadingLWS(std::string& s) {
    size_t leading_whitespace = 0;
    size_t slen = s.size();
    for (size_t i = 0; i < slen; ++i) {
        if (s[i] == ' ' || s[i] == '\t')
            ++leading_whitespace;
    }

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
