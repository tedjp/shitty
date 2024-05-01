#include <cassert>
#include <stdexcept>

#include "../Payload.h"
#include "../StreamBuf.h"
#include "Frame.h"

namespace shitty::http2 {

void FrameHeader::writeTo(Payload& out) const {
    std::array<std::byte, SIZE> bytes = {};
    uint32_t reservedAndStreamId = (static_cast<uint32_t>(reserved) << 31) | streamId;

    bytes[0] = static_cast<std::byte>(length >> 16);
    bytes[1] = static_cast<std::byte>(length >>  8);
    bytes[2] = static_cast<std::byte>(length >>  0);
    bytes[3] = static_cast<std::byte>(type);
    bytes[4] = static_cast<std::byte>(flags.to_ulong());
    bytes[5] = static_cast<std::byte>(reservedAndStreamId >> 24);
    bytes[6] = static_cast<std::byte>(reservedAndStreamId >> 16);
    bytes[7] = static_cast<std::byte>(reservedAndStreamId >>  8);
    bytes[8] = static_cast<std::byte>(reservedAndStreamId >>  0);

    out.write(bytes.data(), bytes.size());
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

    input.advance(FrameHeader::SIZE);

    return header;
}

} // namespace
