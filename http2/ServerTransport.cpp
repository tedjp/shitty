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

private:
    Connection* connection_;
    const Routes* routes_;
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

ServerTransport::Impl::Impl(
        Connection* connection,
        string_view settings,
        const Routes* routes):
    connection_(connection),
    routes_(routes)
{}

void ServerTransport::Impl::onInput(StreamBuf&) {
    // TODO
}

} // namespace
