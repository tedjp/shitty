#pragma once

#include <array>

namespace shitty {

// Provides quick access to 3-char ASCII C-strings of HTTP status codes.
// Because std::to_string(int) is really slow.
//
// This class is thread-safe. You should just reuse the globally-available
// instance `shitty::status_strings`.
class StatusStrings {
public:
    StatusStrings();

    const char* get(unsigned code) const __attribute__((const));
    const char* operator()(unsigned status_code) const __attribute__((const)) {
        return get(status_code);
    }
    const char* operator[](unsigned status_code) const __attribute__((const)) {
        return get(status_code);
    }

private:
    std::array<char[4], 500> strs_;
};

extern const StatusStrings status_strings;

}
