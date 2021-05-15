#include <cassert>

#include "../http2/ServerTransport.h" // for upgrades
#include "HTTP1.h"
#include "HTTP2Upgrader.h"
#include "ServerTransport.h"
#include "UpgradeRegistry.h"

using namespace std;
using namespace shitty::http1;
using shitty::Response;

namespace {

struct Upgrades {
    UpgradeRegistry registry;

    Upgrades() {
        // Uncomment to test Upgrade handling code path with a fake "h1c"
        // protocol that creates a new HTTP/1.1 transport.
        //registry.add("h1c", make_unique<TestHTTP1Upgrader>());
        registry.add("h2c", make_unique<HTTP2Upgrader>());
    }
};

Upgrades staticUpgrades;

} // anonymous

ServerTransport::ServerTransport(
        Connection* connection,
        const Routes* routes):
    shitty::http1::Transport(connection),
    routes_(routes)
{
}

void ServerTransport::sendResponse(const Response& resp) {
    sendMessage(statusLine(resp), resp.message);
    request_handler_ = nullptr;
}

void ServerTransport::handleIncomingMessage(IncomingMessage&& msg) {
    auto request_line = parseRequestLine(std::move(msg.first_line));
    onRequest(Request(request_line.method, request_line.path, std::move(msg.message)));
}

void ServerTransport::onRequest(Request&& req) {
    if (tryUpgrade(req))
        return;

    request_handler_ = routes_->getHandler(req);

    if (!request_handler_) {
        sendResponse(Response(404, "No handler"));
        return;
    }

    request_handler_->onRequest(std::move(req), this);
}

void ServerTransport::onEndOfMessageHeaders(Headers& headers) {
    const Header& expectHeader = headers.get("expect");
    if (expectHeader != no_header) {
        handleExpect(expectHeader.second);
        return;
    }
}

// Upgrades to a new transport: replaces `this` as this connection's Transport,
// then invokes the request handler with the pre-upgrade request.
void ServerTransport::upgrade(
        const string& token,
        unique_ptr<shitty::Transport>&& newTransport,
        Request&& request) {
    assert(newTransport != nullptr);

    {
        Headers headers;
        headers.add("Connection", "Upgrade");
        headers.set("Upgrade", token);
        // 101 Switching Protocols
        sendHeaders(statusLine(101), move(headers));
    }

    Connection* connection = getConnection();
    connection->setTransport(move(newTransport));
    // `this` has been destroyed, but this stack frame still exists
    http2::ServerTransport* h2Transport = dynamic_cast<http2::ServerTransport*>(
            connection->getTransport());
    assert(h2Transport != nullptr);
    // The upgraded incoming request becomes stream 1.
    http2::ServerStream* stream = h2Transport->getStream(1);
    assert(stream != nullptr);
    stream->onRequest(move(request));
}

bool ServerTransport::tryUpgrade(Request& request) {
    // TODO: Check that there is no more data on the connection;
    // a client that pipelines multiple Upgrade requests should have its
    // Upgrade requests ignored.

    const Header& upgradeHeader = request.headers().get("upgrade");
    if (upgradeHeader == no_header)
        return false;

    // FIXME: Tokenize upgradeHeader and try each token individually.
    // This assumes only a single upgrade token, which is incorrect.
    string token = upgradeHeader.second;

    const Upgrader* upgrader = staticUpgrades.registry.get(token);
    if (!upgrader)
        return false;

    unique_ptr<shitty::Transport> transport = upgrader->upgrade(this, request);

    if (!transport)
        return false;

    // Delete the Upgrade header to avoid multiple upgrades.
    // XXX: Delete "Upgrade" from the "Connection" header too for completeness.
    request.headers().remove("Upgrade");

    upgrade(token, move(transport), move(request));
    // `this` transport has been destroyed.

    return true;
}

// This SHALL be overridden by proxies, which, for HTTP/1.1 and later clients,
// are required to forward the request to the origin.
void ServerTransport::handleExpect(const std::string& value) {
    // TODO: If client is HTTP/1.0 or earlier, MUST ignore the Expect header.

    if (value != "100-continue") {
        Response response(417, "Expectation Failed.\n");
        sendMessage(statusLine(response), response.message);
        return;
    }

    sendResponse(Response(100, Message()));
}
