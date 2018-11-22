#pragma once

#include "ClientTransportSource.h"
#include "Handler.h"

namespace shitty {

class ProxyHandler: public Handler {
public:
    ProxyHandler(ClientTransportSource *client_transport_source);
    ProxyHandler(const ProxyHandler&) = default;
    ProxyHandler(ProxyHandler&&) = default;
    ProxyHandler& operator=(const ProxyHandler&) = default;
    ProxyHandler& operator=(ProxyHandler&&) = default;
    virtual ~ProxyHandler() = default;

    void handle(Request&& request, ServerTransport *transport) override;

    void sendBackendRequest(Request&&);
    void respond(Response&&);

protected:
    virtual void onRequest(Request&&);
    virtual void onBackendResponse(Response&&);

    void acquireBackendTransport(const Request& request);
    void releaseBackendTransport();

private:
    ServerTransport* front_transport_;

    ClientTransportSource* client_transport_source_;
    ClientTransport* backend_transport_;
};

}
