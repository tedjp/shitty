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

    // Permit short requests like "GET / HTTP/1.1\r\n\r\n" that are shorter
    // than the HTTP/2 client preface. As soon as a request like that is
    // received, it should switch to HTTP/1 handling.
    bool maybeHTTP2 = true;
    bool definitelyHTTP2 = false;

    if (buf.size() < CLIENT_PREFACE.size()) {
        maybeHTTP2 = memcmp(buf.data(), CLIENT_PREFACE.data(), buf.size()) == 0;

        if (maybeHTTP2)
            return; // wait for more data to be sure
    } else {
        definitelyHTTP2 = memcmp(
                buf.data(),
                CLIENT_PREFACE.data(),
                CLIENT_PREFACE.size()) == 0;
    }

    // `this` will be destroyed in the middle of this function, so grab a
    // frame-local copy of the connection pointer.
    Connection* connection = connection_;
    connection_ = nullptr;

    unique_ptr<Transport> transport;

    if (definitelyHTTP2)
        transport = make_unique<http2::ServerTransport>(connection);
    else
        transport = make_unique<http1::ServerTransport>(connection);

    connection->setTransport(move(transport));
    // `this` has been destroyed: no more access to class members!

    // Invoke real transport with the input so far
    connection->getTransport()->onInput(buf);
}

} // namespace
