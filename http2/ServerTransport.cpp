#include <unordered_map>

#include "ServerStream.h"
#include "ServerTransport.h"

using namespace std;

namespace shitty::http2 {

class ServerTransport::Impl {
public:
    Impl(
            Connection* connection,
            string_view settings,
            const Routes* routes);

    void onInput(StreamBuf& buf);

    ServerStream* getStream(uint32_t id);

private:
    Connection* connection_;
    const Routes* routes_;
    // unique_ptr indirection here is so that ServerStream
    // pointers/references are not invalidated by other streams being created or
    // destroyed in the same container (potential rehashing causing values to
    // move).
    unordered_map<uint32_t, unique_ptr<ServerStream>> streams_;
};

ServerTransport::ServerTransport(
        Connection* connection,
        string_view settings,
        const Routes* routes):
    impl_(make_unique<Impl>(connection, settings, routes))
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
        string_view settings,
        const Routes* routes):
    connection_(connection),
    routes_(routes)
{
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
