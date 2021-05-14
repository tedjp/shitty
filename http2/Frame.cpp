#include <arpa/inet.h>
#include <cassert>
#include <stdexcept>

#include "../Payload.h"
#include "../StreamBuf.h"
#include "Frame.h"

namespace shitty::http2 {

const FrameHeader DataFrameHeader = FrameHeader{ 0, FrameId::DATA };
const FrameHeader HeadersFrameHeader = FrameHeader{ 0, FrameId::HEADERS };
const FrameHeader PriorityFrameHeader = FrameHeader{ 0, FrameId::PRIORITY };
const FrameHeader ResetStreamFrameHeader = FrameHeader{ 0, FrameId::RST_STREAM };
const FrameHeader SettingsFrameHeader = FrameHeader{ 0, FrameId::SETTINGS };
const FrameHeader PushPromiseFrameHeader = FrameHeader{ 0, FrameId::PUSH_PROMISE };
const FrameHeader PingFrameHeader = FrameHeader{ 0, FrameId::PING };
const FrameHeader GoAwayFrameHeader = FrameHeader{ 0, FrameId::GOAWAY };
const FrameHeader WindowUpdateFrameHeader = FrameHeader{ 0, FrameId::WINDOW_UPDATE };
const FrameHeader ContinuationFrameHeader = FrameHeader{ 0, FrameId::CONTINUATION };

// XXX: This ought to be inlined and ideally a single write of a packed
// structure.
void writeFrameHeader(const FrameHeader& frameHeader, Payload& out) {
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >> 16));
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >>  8));
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >>  0));
    out.writeOctet(frameHeader.type);
    out.writeOctet(frameHeader.flags);
    uint32_t nboReservedStreamId = htonl(
            (frameHeader.reserved << 31) | frameHeader.streamId);
    out.write(&nboReservedStreamId, sizeof(nboReservedStreamId));
}

FrameHeader readFrameHeader(StreamBuf& input) {
    FrameHeader header;

    if (input.size() < 9)
        throw std::runtime_error("incomplete header"); // XXX: Maybe it hasn't arrived yet. call back.

    header.length
        = static_cast<uint32_t>(input.data()[0]) << 16
        | static_cast<uint32_t>(input.data()[1]) <<  8
        | static_cast<uint32_t>(input.data()[2]) <<  0;

    header.type = input.data()[3];
    header.flags = input.data()[4];

    uint32_t reservedAndStreamId
        = static_cast<uint32_t>(input.data()[5]) << 24
        | static_cast<uint32_t>(input.data()[6]) << 16
        | static_cast<uint32_t>(input.data()[7]) <<  8
        | static_cast<uint32_t>(input.data()[8]) <<  0;

    header.reserved = reservedAndStreamId >> 31;
    header.streamId = reservedAndStreamId & 0x7fffffffu;

    return header;
}

} // namespace
