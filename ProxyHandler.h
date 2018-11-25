#pragma once

#include "ClientTransportSource.h"
#include "RequestHandlerFactory.h"

namespace shitty {

class ProxyHandler: public RequestHandler {
public:
    ProxyHandler(ClientTransportSource *client_transport_source);

    ProxyHandler(ProxyHandler&&) = default;
    ProxyHandler& operator=(ProxyHandler&&) = default;

    ProxyHandler(const ProxyHandler&) = delete;
    ProxyHandler& operator=(const ProxyHandler&) = delete;

    virtual ~ProxyHandler() = default;

    void onRequest(Request&&, ServerTransport *transport) override;
    void sendBackendRequest(Request&&);
    void respond(Response&&);

protected:
    virtual void onBackendResponse(Response&&);

private:
    void acquireBackendTransport(const Request&);
    void releaseBackendTransport();

    ClientTransportSource* client_transport_source_;

    ServerTransport* front_transport_;
    ClientTransport* backend_transport_;
};

class ProxyHandlerFactory: public RequestHandlerFactory {
public:
    ProxyHandlerFactory(ClientTransportSource* client_transport_source):
        client_transport_source_(client_transport_source)
    {}

    std::unique_ptr<RequestHandler> getHandler() const override {
        return std::make_unique<ProxyHandler>(client_transport_source_);
    }

private:
    ClientTransportSource* client_transport_source_;
};

}
