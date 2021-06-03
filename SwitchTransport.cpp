#include "Connection.h"
#include "SwitchTransport.h"

#include "http1/ServerTransport.h"
#include "http2/Protocol.h"
#include "http2/ServerTransport.h"

using namespace std;

namespace shitty {

SwitchTransport::SwitchTransport(Connection* connection):
    connection_(connection)
{}

void SwitchTransport::onInput(StreamBuf& buf) {
    using http2::protocol::CLIENT_PREFACE;

    if (buf.size() < CLIENT_PREFACE.size())
        return; // wait for more

    const bool isHttp2 = memcmp(
            buf.data(),
            CLIENT_PREFACE.data(),
            CLIENT_PREFACE.size()) == 0;

    // `this` will be destroyed in the middle of this function, so grab a
    // frame-local copy of the connection pointer.
    Connection* connection = connection_;
    connection_ = nullptr;

    unique_ptr<Transport> transport;

    if (isHttp2)
        transport = make_unique<http2::ServerTransport>(connection);
    else
        transport = make_unique<http1::ServerTransport>(connection);

    connection->setTransport(move(transport));
    // `this` has been destroyed: no more access to class members!

    // Invoke real transport with the input so far
    connection->getTransport()->onInput(buf);
}

} // namespace
