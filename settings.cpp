#include "settings.h"

namespace shitty::http2 {

ErrorCode Settings::validate(Setting setting, uint32_t value) {
    if (value == 0 || value > initial_values.size()) {
        // Must ignore unrecognized setting
        return NO_ERROR;
    }

    switch (setting) {
    case HEADER_TABLE_SIZE:
        return NO_ERROR;

    case ENABLE_PUSH:
        if (value == 1 || value == 0)
            return NO_ERROR;
        return PROTOCOL_ERROR;

    case MAX_CONCURRENT_STREAMS:
        return NO_ERROR;

    case INITIAL_WINDOW_SIZE:
        if (value < (1ul << 31) - 1)
            return NO_ERROR;
        return FLOW_CONTROL_ERROR;

    case MAX_FRAME_SIZE:
        if (value >= (1 << 14) || value <= (1 << 24) - 1)
            return NO_ERROR;
        return PROTOCOL_ERROR;

    case MAX_HEADER_LIST_SIZE:
        return NO_ERROR;
    }

    throw std::logic_error("Unrecognized setting not ignored");
}

void Settings::set(Setting setting, uint32_t value) {
    ErrorCode e = validate(setting, value);
    if (e != NO_ERROR)
        throw e;

    if (setting < 1 || setting > values_.size()) {
        // ignore
        return;
    }

    values_[setting - 1] = value;
}

constexpr std::array<uint32_t, 6>
shitty::http2::Settings::initial_values;

} // namespace
