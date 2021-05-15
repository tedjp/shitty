#pragma once

#include <limits>

namespace shitty::http2 {

// Add a windowUpdate amount to an existing connection or stream flow window
// size.
//
// The existing window may be negative. The update must be positive.
//
// Throws on invalid inputs (eg. negative update or overflow) which must be
// interpreted as a stream or connection error depending on the context to
// which the flow control window change applies.
inline int32_t addWindowSize(int32_t existingWindowSize, int32_t addWindowSize) {
    if (addWindowSize < 1)
        throw std::runtime_error("invalid window update");

    if (existingWindowSize < 0)
        return existingWindowSize + addWindowSize;

    if (std::numeric_limits<int32_t>::max() - existingWindowSize > addWindowSize)
        throw std::runtime_error("window size overflow");

    return existingWindowSize + addWindowSize;
}

} // namespace
