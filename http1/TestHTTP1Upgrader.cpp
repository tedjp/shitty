#include "ServerTransport.h"
#include "TestHTTP1Upgrader.h"

using namespace std;

namespace shitty::http1 {

const Routes TestHTTP1Upgrader::routes_;

unique_ptr<shitty::Transport> TestHTTP1Upgrader::upgrade(http1::Transport* transport) const {
    return make_unique<http1::ServerTransport>(transport->getConnection(), &routes_);
}

} // namespace
