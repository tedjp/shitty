#include <functional>

#include "Message.h"
#include "BufferMessageHandler.h"

namespace shitty {

// Message handler constructed with a functor that is invoked once when the
// entire message (request or response) has been received.
class SimpleMessageHandler : public BufferMessageHandler {
public:
    using Handler = std::function<void(Message&&)>;

    explicit SimpleMessageHandler(Handler&& handler);

    void onEndOfMessage(std::optional<Headers> optTrailers) override;

    void onAllInOne(
            Headers&& headers,
            std::span<const std::byte> body,
            std::optional<Headers>&& optTrailers) override;

private:
    // Target handler function invoked when the request/response is complete.
    Handler handler_;
};

} // namespace shitty
