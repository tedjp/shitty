#pragma once

#include <limits>
#include <string>

#include "../ServerStream.h"
#include "FlowControl.h"

namespace shitty::http2 {

class ServerTransport;

enum class StreamState {
    Idle,
    ReservedLocal,
    ReservedRemote,
    Open,
    HalfClosedLocal,
    HalfClosedRemote,
    Closed,
};

class ServerStream: public shitty::ServerStream {
public:
    ServerStream(
            uint32_t id,
            ServerTransport* transport);

    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;

    StreamState getState() const;
    void setState(StreamState);

    void addWindowSize(int32_t windowSize);
    void subtractWindowSize(uint32_t adjustment);

    int32_t availableWindowSize() const;

    const Headers& headers() const;

private:
    // Stream Identifier (RFC 7540 5.1.1; unsigned 31-bit).
    uint32_t id_ = 0;

    // Non-owning pointer to the (parent) transport. Useful for sending
    // responses.
    // TODO: Hold a pointer to the Impl instead to avoid an indirection.
    ServerTransport* transport_ = nullptr;

    // initial value from RFC 7540 6.5.2.
    int32_t windowSize_ = 65535;

    StreamState state_ = StreamState::Idle;

    Headers headers_;

    // Buffered body
    std::vector<std::byte> body_;
};

inline StreamState ServerStream::getState() const {
    return state_;
}

inline void ServerStream::setState(StreamState state) {
    state_ = state;
}

inline void ServerStream::addWindowSize(int32_t windowSize) {
    try {
        windowSize_ = shitty::http2::addWindowSize(windowSize_, windowSize);
    } catch (std::runtime_error& err) {
        // FIXME: This is a PROTOCOL_ERROR (section 6.9)
        // that should trigger a RST_STREAM.
        throw std::runtime_error(std::string("stream error: ") + err.what());
    }
}

inline void ServerStream::subtractWindowSize(uint32_t adjustment) {
    static_assert(std::is_same_v<decltype(windowSize_), int32_t>);

    // Negative window size is OK, but overflow is not.
    if ((windowSize_ < 0 && adjustment > abs(windowSize_))
        || adjustment > std::numeric_limits<int32_t>::max())
        throw std::runtime_error("window size reduction too large");

    windowSize_ -= static_cast<int32_t>(adjustment);
}

inline int32_t ServerStream::availableWindowSize() const {
    return windowSize_;
}

inline const Headers& ServerStream::headers() const {
    return headers_;
}

} // namespace
