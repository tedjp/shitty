#include "BufferMessageHandler.h"

namespace shitty {

class BufferMessageHandler : public MessageHandler {
public:
    void onHeaders(Headers&& headers) override;
    void onBodyPart(std::span<const std::byte> bodyPart) override;

protected:
    Headers headers_;
    std::vector<std::byte> body_;
};

void BufferMessageHandler::onHeaders(Headers&& headers) {
    headers_ = std::move(headers);
}

void BufferMessageHandler::onBodyPart(std::span<const std::byte> bodyPart) {
    body_.append(bodyPart);
}

} // namespace shitty
