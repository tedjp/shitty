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

    // May use the request, but shall not respond.
    // The Transport's onRequest function will be invoked later with ownership
    // of the Request.
    virtual std::unique_ptr<shitty::Transport> upgrade(
            http1::Transport* existingTransport,
            const Request& request) const = 0;
};

} // namespace
