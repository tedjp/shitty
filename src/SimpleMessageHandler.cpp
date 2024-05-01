#include "SimpleMessageHandler.h"

namespace shitty {

void SimpleMessageHandler::onEndOfMessage(std::optional<Headers> optTrailers) {
    if (optTrailers)
        headers_ += optTrailers.value();

    handler_(Message(std::move(headers_), body_));
}

} // namespace shitty
