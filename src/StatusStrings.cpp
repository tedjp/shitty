#include <stdexcept>

#include "StatusStrings.h"

using shitty::StatusStrings;

StatusStrings::StatusStrings() {
    for (unsigned code = 100; code < 600; ++code) {
        std::string codestr = std::to_string(code);
        char *str = strs_[code - 100];
        str[0] = codestr[0];
        str[1] = codestr[1];
        str[2] = codestr[2];
        str[3] = '\0';
    }
}

// This function is __attribute__((const))
const char *
StatusStrings::get(unsigned code) const {
    if (code < 100 || code > 599)
        throw std::range_error("invalid status code");

    return strs_[code - 100];
}

namespace shitty {

const StatusStrings status_strings = StatusStrings();

}
