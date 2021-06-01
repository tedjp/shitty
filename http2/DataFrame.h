#include "Frame.h"

namespace shitty::http2 {

class DataFrame {
public:
    static bool isEndStream(const FrameHeader& frameHeader);
    static bool isPadded(const FrameHeader& frameHeader);
};

inline bool DataFrame::isEndStream(const FrameHeader& frameHeader) {
    return frameHeader.flags.test(3);
}

inline bool DataFrame::isPadded(const FrameHeader& frameHeader) {
    return frameHeader.flags.test(0);
}

} // namespace
