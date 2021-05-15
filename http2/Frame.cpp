#include <arpa/inet.h>
#include <cassert>
#include <stdexcept>

#include "../Payload.h"
#include "../StreamBuf.h"
#include "Frame.h"

namespace shitty::http2 {

// XXX: This ought to be inlined and ideally a single write of a packed
// structure.
void writeFrameHeader(const FrameHeader& frameHeader, Payload& out) {
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >> 16));
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >>  8));
    out.writeOctet(static_cast<uint8_t>(frameHeader.length >>  0));
    out.writeOctet(static_cast<uint8_t>(frameHeader.type));
    out.writeOctet(static_cast<uint8_t>(frameHeader.flags.to_ulong()));
    uint32_t nboReservedStreamId = htonl(
            (frameHeader.reserved << 31) | frameHeader.streamId);
    out.write(&nboReservedStreamId, sizeof(nboReservedStreamId));
}

std::optional<FrameHeader> tryReadFrameHeader(StreamBuf& input) {
    if (input.size() < FrameHeader::SIZE)
        return std::nullopt;

    return readFrameHeader(input);
}

FrameHeader readFrameHeader(StreamBuf& input) {
    if (input.size() < FrameHeader::SIZE)
        throw std::runtime_error("incomplete header");

    FrameHeader header(FrameType(input.data()[3]));

    header.length
        = static_cast<uint32_t>(input.data()[0]) << 16
        | static_cast<uint32_t>(input.data()[1]) <<  8
        | static_cast<uint32_t>(input.data()[2]) <<  0;

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
