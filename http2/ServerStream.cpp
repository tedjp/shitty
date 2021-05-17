#include <array>
#include <stdexcept>

#include "../Response.h"
#include "HeadersFrame.h"
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
    // TODO.. dispatch to request handler
    sendResponse(Response(200));
}

void ServerStream::sendResponse(const Response&) {
    HeadersFrame headers(id_);
    // static table (0x80) | ":status = 200" (0x08)
    headers.setHeaderBlockFragment({std::byte(0x88)});
    // This completes the response.
    headers.setEndStream(true);

    transport_->writeHeadersFrame(std::move(headers));
}

} // namespace
