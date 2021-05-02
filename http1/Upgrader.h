#pragma once

#include "Transport.h"

namespace shitty::http1 {

class Upgrader {
public:
    Upgrader() = default;
    Upgrader(const Upgrader&) = default;
    Upgrader(Upgrader&&) = default;
    Upgrader& operator=(const Upgrader&) = default;
    Upgrader& operator=(Upgrader&&) = default;
    virtual ~Upgrader() = default;

    virtual std::unique_ptr<shitty::Transport> upgrade(http1::Transport* existingTransport) const = 0;
};

} // namespace
