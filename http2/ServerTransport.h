#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include "../Transport.h"
#include "Frame.h"
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
    // settings - base64-encoded HTTP2-Settings header
    // routes - routes
    ServerTransport(
            Connection* connection,
            const Header& http2Settings,
            const Routes* routes);
    ~ServerTransport();

    // shitty::Transport overrides
    void onInput(StreamBuf&) override;

    void writeFrame(FrameHeader frameHeader, std::span<const std::byte> data);

    ServerStream* getStream(uint32_t id);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace
