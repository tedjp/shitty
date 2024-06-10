#pragma once

#include "../Routes.h"
#include "Upgrader.h"

namespace shitty::http1 {

class HTTP2Upgrader: public Upgrader {
public:
    std::unique_ptr<shitty::Transport> upgrade(
            http1::Transport* existingTransport,
            const Request& request) const override;

private:
    // FIXME: Never initialized to anything useful.
    Server* server_ = nullptr;
};

} // namespace
