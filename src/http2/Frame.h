#pragma once

#include <bitset>
#include <cstdint>
#include <optional>

#include "../Payload.h"
#include "../StreamBuf.h"

namespace shitty::http2 {

enum class FrameType: uint8_t {
    // enum values allcapsed to match spec
    DATA = 0x00,
    HEADERS = 0x01,
    PRIORITY = 0x02,
    RST_STREAM = 0x03,
    SETTINGS = 0x04,
    PUSH_PROMISE = 0x05,
    PING = 0x06,
    GOAWAY = 0x07,
    WINDOW_UPDATE = 0x08,
    CONTINUATION = 0x09,
};

struct FrameHeader {
    static constexpr unsigned SIZE = 9;

    FrameHeader(FrameType typeParam): type(typeParam) {}

    uint32_t length : 24 = 0;
    FrameType type;
    std::bitset<8> flags;
    unsigned reserved : 1 = 0;
    uint32_t streamId : 31 = 0;

    void writeTo(Payload& payload) const;
};

// input *must* have at least FrameHeader::SIZE (9) octets available
FrameHeader readFrameHeader(StreamBuf& input);
// FrameHeader, or nullopt if not enough data available
std::optional<FrameHeader> tryReadFrameHeader(StreamBuf& input);

// Convenience predicates for various standard frame types
// Don't worry about frame-type check duplication -- it'll optimize away.
inline bool isSettingsACK(const FrameHeader& frameHeader) {
    return frameHeader.type == FrameType::SETTINGS && frameHeader.flags.test(0);
}

inline bool isPingACK(const FrameHeader& frameHeader) {
    return frameHeader.type == FrameType::PING && frameHeader.flags.test(0);
}

} // namespace
