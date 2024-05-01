#include "Frame.h"

namespace shitty::http2 {

class DataFrame {
public:
    static bool isEndStream(const FrameHeader& frameHeader);
    static bool isPadded(const FrameHeader& frameHeader);
    static uint32_t padding(const FrameHeader& frameHeader);
};

inline bool DataFrame::isEndStream(const FrameHeader& frameHeader) {
    // https://datatracker.ietf.org/doc/html/rfc7540#page-32
    return frameHeader.flags.test(3);
}

inline bool DataFrame::isPadded(const FrameHeader& frameHeader) {
    // https://datatracker.ietf.org/doc/html/rfc7540#page-32
    return frameHeader.flags.test(0);
}

inline uint32_t DataFrame::padding(const FrameHeader& frameHeader) {
    if (!isPadded(frameHeader))
        return 0;
}

} // namespace
