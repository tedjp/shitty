#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include "../Transport.h"
#include "Frame.h"
#include "HeadersFrame.h"
#include "ServerStream.h"

namespace shitty {
class Connection;
class Routes;
}

namespace shitty::http2 {

class ServerTransport: public shitty::Transport {
public:
    // Construct
    // connection - connection
    // routes - routes
    // settings - base64-encoded HTTP2-Settings header
    ServerTransport(
            Connection& connection,
            const Routes& routes,
            const Header* http2Settings = nullptr);
    ~ServerTransport();

    // shitty::Transport overrides
    void onInput(StreamBuf&) override;

    void writeFrame(FrameHeader frameHeader, std::span<const std::byte> data);
    void writeHeadersFrame(const HeadersFrame& frame);

    ServerStream& getStream(uint32_t id);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace
