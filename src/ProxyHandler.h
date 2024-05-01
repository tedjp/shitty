#pragma once

#include "http1/ClientTransportSource.h"
#include "RequestHandlerFactory.h"

namespace shitty {

class ProxyHandler: public RequestHandler {
public:
    ProxyHandler(http1::ClientTransportSource *client_transport_source);

    ProxyHandler(ProxyHandler&&) = default;
    ProxyHandler& operator=(ProxyHandler&&) = default;

    ProxyHandler(const ProxyHandler&) = delete;
    ProxyHandler& operator=(const ProxyHandler&) = delete;

    virtual ~ProxyHandler() = default;

    void onRequest(Request&&, ServerStream *stream) override;
    void sendBackendRequest(Request&&);
    void respond(Response&&);

protected:
    virtual void onBackendResponse(Response&&);

private:
    void acquireBackendTransport(const Request&);
    void releaseBackendTransport();

    http1::ClientTransportSource* client_transport_source_;

    ServerStream* front_stream_;
    http1::ClientTransport* backend_transport_;
};

using ProxyHandlerFactory = SimpleRequestHandlerFactory<
    ProxyHandler,
    http1::ClientTransportSource*>;

}
