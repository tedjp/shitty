#pragma once

#include "http1/ClientTransportSource.h"
#include "RequestHandler.h"
#include "Responder.h"

namespace shitty {

// Shall be created with `new` and deletes itself once a backend response is received.
// This API is not great and could use rework.
class PerRequestProxyHandler {
public:
    explicit PerRequestProxyHandler(
        http1::ClientTransportSource *client_transport_source,
        Request&& request,
        Responder&& responder);

    void sendBackendRequest(Request&&);
    void respond(Response&&);

protected:
    virtual void onBackendResponse(Response&&);

private:
    void acquireBackendTransport(const Request&);
    void releaseBackendTransport();

    http1::ClientTransportSource* client_transport_source_ = nullptr;

    Responder responder_;
    http1::ClientTransport* backendTransport_ = nullptr;
};

RequestHandler MakeProxyHandler(http1::ClientTransportSource* clientTransportSource);

}
