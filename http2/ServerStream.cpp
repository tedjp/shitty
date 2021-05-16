#include <array>
#include <stdexcept>

#include "ServerStream.h"
#include "ServerTransport.h"

namespace shitty::http2 {

ServerStream::ServerStream(uint32_t id, ServerTransport* transport):
    id_(id),
    transport_(transport)
{
    if (id_ > 0x7fffffff)
        throw std::runtime_error("stream id too big");

    if (transport_ == nullptr)
        throw std::runtime_error("null transport");
}

void ServerStream::onRequest(Request&& request) {
    // TODO
}

void ServerStream::sendResponse(const Response&) {
    // XXX: This implementation is a placeholder
    FrameHeader okHeaders(FrameType::HEADERS);
    okHeaders.streamId = id_;

    // static table ":status = 200".
    const std::array<const std::byte, 1> headers{std::byte(0x88)};

    transport_->writeFrame(std::move(okHeaders), headers);
}

} // namespace
