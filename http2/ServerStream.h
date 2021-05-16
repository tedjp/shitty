#pragma once

#include <limits>
#include <string>

#include "../ServerStream.h"
#include "FlowControl.h"

namespace shitty::http2 {

class ServerTransport;

class ServerStream: public shitty::ServerStream {
public:
    ServerStream(
            uint32_t id,
            ServerTransport* transport);

    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;

    void addWindowSize(int32_t windowSize);

private:
    uint32_t id_ = 0;
    // XXX: Hold a pointer to the Impl instead to avoid an indirection
    ServerTransport* transport_ = nullptr;

    // initial value from RFC 7540 6.5.2.
    int32_t windowSize_ = 65535;
};

inline void ServerStream::addWindowSize(int32_t windowSize) {
    try {
        windowSize_ = shitty::http2::addWindowSize(windowSize_, windowSize);
    } catch (std::runtime_error& err) {
        // FIXME: This is a PROTOCOL_ERROR (section 6.9)
        // that should trigger a RST_STREAM.
        throw std::runtime_error(std::string("stream error: ") + err.what());
    }
}

} // namespace
