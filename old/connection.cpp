#include "connection.h"

namespace shitty::http2 {

static const uint8_t CLIENT_PROLOGUE[] = {
    0x50, 0x52, 0x49, 0x20, 0x2a, 0x20, 0x48, 0x54, // PRI * HT
    0x54, 0x50, 0x2f, 0x32, 0x2e, 0x30, 0x0d, 0x0a, // TP/2.0\r\n
    0x0d, 0x0a, 0x53, 0x4d, 0x0d, 0x0a, 0x0d, 0x0a, // \r\nSM\r\n\r\n
};

void Connection::onData() {
    if (state_ == State::BAD) {
        // XXX: Do something? Throw? Close?
#if 0
        if (socket_) {
            // XXX: Send GOAWAY?
            socket_.close();
        }
#endif

        return;
    }

    if (state_ == State::UNCONFIRMED) {
        // 0. Receive prologue.
        // 1. Construct input buffer with default size
        // (construct one on-stack)
        // 2. Read in settings using stack IOBuf
        // 3. adjust input buffer size
        // (resize & move stack IOBuf into input_)
    }
}
}
