#include <cstddef>
#include <span>
#include <vector>

#include "MessageHandler.h"

namespace shitty {

// MessageHandler that buffers body parts. MessageHandler::onEndOfMessage()
// must still be overridden to provide request handling.
class BufferMessageHandler : public MessageHandler {
public:
    void onHeaders(Headers&& headers) override;
    void onBodyPart(std::span<const std::byte> bodyPart) override;

protected:
    Headers headers_;
    std::vector<std::byte> body_;
};

} // namespace shitty
