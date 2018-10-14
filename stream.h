#include "frame.h"

namespace shitty::http2 {
class Stream {
public:
    void onFrame(Frame& frame);
};
} // namespace shitty::http2
