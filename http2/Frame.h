#pragma once

#include <cstdint>

#include "../Payload.h"
#include "../StreamBuf.h"

namespace shitty::http2 {

// Like `enum class` but without the need to static_cast<> to the underlying
// value. Used in the FrameHeader.type field.
struct FrameId {
    enum {
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
};

struct FrameHeader {
    static constexpr unsigned SIZE = 9;

    uint32_t length : 24 = 0;
    uint8_t type = 0;
    uint8_t flags = 0;
    unsigned reserved : 1 = 0;
    uint32_t streamId : 31 = 0;
};

// Convenience default frames that come with the type set. Other fields can be
// filled as required.
extern const FrameHeader DataFrameHeader;
extern const FrameHeader HeadersFrameHeader;
extern const FrameHeader PriorityFrameHeader;
extern const FrameHeader ResetStreamFrameHeader;
extern const FrameHeader SettingsFrameHeader;
extern const FrameHeader PushPromiseFrameHeader;
extern const FrameHeader PingFrameHeader;
extern const FrameHeader GoAwayFrameHeader;
extern const FrameHeader WindowUpdateFrameHeader;
extern const FrameHeader ContinuationFrameHeader;

FrameHeader readFrameHeader(StreamBuf& input);
void writeFrameHeader(const FrameHeader& frameHeader, Payload& payload);

} // namespace
