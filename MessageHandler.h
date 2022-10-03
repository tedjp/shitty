#include <cstddef> // std::byte
#include <span>

#include "Headers.h"

namespace shitty {

// Request or response handler interface.
// A simple handler would buffer the entire request before dispatching, but
// be wary of memory exhaustion by malicious endless requests/responses.
// A more-featured handler might handle the message in chunks.
class MessageHandler {
public:
    virtual void onHeaders(Headers&& headers) {}
    virtual void onBodyPart(std::span<const std::byte>) {}
    virtual void onEndOfMessage(std::optional<Headers> trailers) = 0;

    // Special call used when the entire request or response arrives all at
    // once. May be overridden to avoid copying the message body.
    virtual void onAllInOne(
            Headers&& headers,
            std::span<const std::byte> body,
            std::optional<Headers>&& trailers);
};

} // namespace shitty
