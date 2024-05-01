#pragma once

#include "../Message.h"

namespace shitty::http1 {

/* A container for storing an incoming request-or-response that is agnostic to
 * the first line, which may be a Request-Line or a Status-Line, followed by an
 * HTTP message (headers & optional body). Trailers not yet handled. */
struct IncomingMessage {
    std::string first_line;
    Message message;
};

}
