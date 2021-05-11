#include <unordered_map>

#include <fb64.h> // https://github.com/tedjp/fb64

#include "Settings.h"
#include "ServerStream.h"
#include "ServerTransport.h"

using namespace std;

namespace shitty::http2 {

class ServerTransport::Impl {
public:
    Impl(
            Connection* connection,
            const Header& http2Settings,
            const Routes* routes);

    void onInput(StreamBuf& buf);

    ServerStream* getStream(uint32_t id);

private:
    Connection* connection_;
    Settings settings_;
    const Routes* routes_;
    // unique_ptr indirection here is so that ServerStream
    // pointers/references are not invalidated by other streams being created or
    // destroyed in the same container (potential rehashing causing values to
    // move).
    unordered_map<uint32_t, unique_ptr<ServerStream>> streams_;
};

ServerTransport::ServerTransport(
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    impl_(make_unique<Impl>(connection, http2Settings, routes))
{}

void ServerTransport::onInput(StreamBuf& buf) {
    impl_->onInput(buf);
}

ServerStream* ServerTransport::getStream(uint32_t id) {
    return impl_->getStream(id);
}

ServerTransport::~ServerTransport()
{}

ServerTransport::Impl::Impl(
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    connection_(connection),
    routes_(routes)
{
    // decode settings
    {
        string_view settings = http2Settings.second;
        char decoded[128];
        const size_t decodedLen = fb64_decoded_size_nopad(settings.size());
        if (decodedLen > sizeof(decoded))
            throw runtime_error("excessive HTTP2-Settings length");

        int decodeResult = fb64_decode(
                settings.data(),
                settings.size(),
                reinterpret_cast<uint8_t*>(decoded));
        if (decodeResult != 0)
            throw runtime_error("HTTP2-Settings decode error");

        settings_ = Settings::createFromBuffer(span(decoded, decodedLen));
    }

    streams_.emplace(1, make_unique<ServerStream>());
}

void ServerTransport::Impl::onInput(StreamBuf&) {
    // TODO
}

ServerStream* ServerTransport::Impl::getStream(uint32_t id) {
    auto it = streams_.find(id);
    if (it != streams_.end())
        return it->second.get();

    return nullptr;
}

} // namespace
