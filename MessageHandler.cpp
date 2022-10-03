#include "MessageHandler.h"

namespace shitty {

void MessageHandler::onAllInOne(
        Headers&& headers,
        std::span<const std::byte> body,
        std::optional<Headers>&& trailers)
{
    onHeaders(std::move(headers));
    onBodyPart(std::move(body));
    onEndOfMessage(std::move(trailers));
}

} // namespace shitty
