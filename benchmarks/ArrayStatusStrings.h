#pragma once

#include <array>
#include <string>

namespace shitty::benchmark {

class ArrayStatusStrings {
public:
    ArrayStatusStrings();

    const std::string& get(unsigned code) const;

private:
    std::array<std::string, 500> strs;
};

class ContinuousStatusStrings {
public:
    ContinuousStatusStrings();

    // Return the 3-char status string for a code in the range [100,599].
    // All status strings are NUL-terminated (C-strings).
    // Values outside that range throw an error.
    // Thread-safe.
    const char* str(unsigned status_code) const __attribute__((const));
    const char* operator()(unsigned status_code) const __attribute__((const)) {
        return str(status_code);
    }
    const char* operator[](unsigned status_code) const __attribute__((const)) {
        return str(status_code);
    }

private:
    char strs_[500 * 4];
};

extern const ArrayStatusStrings array_status_strings;
extern const ContinuousStatusStrings continuous_status_strings;

}
