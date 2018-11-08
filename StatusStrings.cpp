#include <stdexcept>

#include "StatusStrings.h"

using shitty::StatusStrings;

StatusStrings::StatusStrings() {
    const char digits[10] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };

    for (unsigned i = 100; i < 600; ++i) {
        strs_[(i - 100) * 4] = digits[i / 100];
        strs_[(i - 100) * 4 + 1] = digits[i % 100 / 10];
        strs_[(i - 100) * 4 + 2] = digits[i % 10];
        strs_[(i - 100) * 4 + 3] = '\0';
    }
}

// This function is __attribute__((const))
const char* StatusStrings::str(unsigned status_code) const {
    if (status_code < 100 || status_code > 599)
        throw std::runtime_error("Invalid HTTP status code");

    return &strs_[(status_code - 100) * 4];
}

namespace shitty {
const StatusStrings status_strings;
}
