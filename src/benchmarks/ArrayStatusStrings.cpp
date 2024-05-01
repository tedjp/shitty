#include "ArrayStatusStrings.h"

namespace shitty::benchmark {

ArrayStatusStrings::ArrayStatusStrings() {
    for (unsigned code = 100; code < 600; ++code) {
        strs[code - 100] = std::to_string(code);
    }
}

const std::string& ArrayStatusStrings::get(unsigned code) const {
    if (code < 100 || code > 599)
        throw std::range_error("invalid status code");

    return strs[code - 100];
}

ContinuousStatusStrings::ContinuousStatusStrings() {
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

const char* ContinuousStatusStrings::str(unsigned status_code) const {
    if (status_code < 100 || status_code > 599)
        throw std::runtime_error("Invalid HTTP status code");

    return &strs_[(status_code - 100) * 4];
}

const ArrayStatusStrings array_status_strings = ArrayStatusStrings();
const ContinuousStatusStrings continuous_status_strings = ContinuousStatusStrings();

}
