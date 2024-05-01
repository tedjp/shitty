#pragma once

#include <array>
#include <cstdint>

#include "error.h"

namespace shitty::http2 {

class Settings/*: public Frame */ {
public:
    enum Setting {
        HEADER_TABLE_SIZE = 0x1,
        ENABLE_PUSH = 0x2,
        MAX_CONCURRENT_STREAMS = 0x3,
        INITIAL_WINDOW_SIZE = 0x4,
        MAX_FRAME_SIZE = 0x5,
        MAX_HEADER_LIST_SIZE = 0x6,
    };

    static ErrorCode validate(Setting setting, uint32_t value);

    void set(Setting setting, uint32_t value);
    uint32_t get(Setting setting) const {
        return values_[setting];
    }

private:
    static constexpr std::array<uint32_t, 6> initial_values = {
        4096,
        1,
        UINT32_MAX,
        65535,
        16384,
        UINT32_MAX,
    };

    std::array<uint32_t, 6> values_ = initial_values;
};

} // namespace shitty::http2
