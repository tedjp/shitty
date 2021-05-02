#pragma once

#include "../Routes.h"
#include "Upgrader.h"

namespace shitty::http1 {

// An upgrader that creates a new HTTP/1.1 transport (for testing).
// Remove this once an h2c (HTTP/2) Upgrader is available.
class TestHTTP1Upgrader: public Upgrader {
public:
    std::unique_ptr<shitty::Transport> upgrade(http1::Transport* existingTransport) const override;

private:
    // empty routes since this is just for test :3
    static const Routes routes_;
};

} // namespace
