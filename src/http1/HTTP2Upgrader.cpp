#include "../http2/ServerTransport.h"
#include "HTTP2Upgrader.h"

using namespace std;

namespace shitty::http1 {

unique_ptr<shitty::Transport> HTTP2Upgrader::upgrade(
        http1::Transport* transport,
        const Request& request) const
{
    return make_unique<http2::ServerTransport>(
            transport->getConnection(),
            *server_,
            &request.headers().get("HTTP2-Settings"));
}

} // namespace
