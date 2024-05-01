#pragma once

#include "Transport.h"

namespace shitty {

class Connection;
class Routes;

// Inspects the beginning of a client's communication to determine whether
// HTTP/2 or HTTP/1 is in use. HTTP/2 may be started immediately by way of Prior
// Knowledge. Once determined, replaces itself as the Connection's transport and
// re-invokes the real transport's onInput() function.
class SwitchTransport: public Transport {
public:
    SwitchTransport(Connection& connection, const Routes& routes);

    void onInput(StreamBuf& buf) override;

private:
    Connection* connection_;
    const Routes* routes_;
};

}
