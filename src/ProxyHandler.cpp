#include "ProxyHandler.h"
#include "http1/ServerTransport.h"

namespace shitty {

PerRequestProxyHandler::PerRequestProxyHandler(
    http1::ClientTransportSource* source,
    Request&& request,
    Responder&& responder):
    client_transport_source_(source),
    responder_(std::move(responder))
{
    sendBackendRequest(std::move(request));
}

void PerRequestProxyHandler::sendBackendRequest(Request&& request) {
    acquireBackendTransport(request);
    backendTransport_->sendRequest(std::move(request));
}

void PerRequestProxyHandler::respond(Response&& response) {
    responder_.respond(std::move(response));
}

void PerRequestProxyHandler::onBackendResponse(Response&& response) {
    releaseBackendTransport();
    respond(std::move(response));
    delete this;
}

void
PerRequestProxyHandler::acquireBackendTransport(const Request& request) {
    backendTransport_ = client_transport_source_->getTransport(
        [this](Response&& response, ClientStream*) {
            onBackendResponse(std::move(response));
        });
}

void
PerRequestProxyHandler::releaseBackendTransport() {
    backendTransport_->resetHandler();
    client_transport_source_->putTransport(std::move(backendTransport_));
    backendTransport_ = nullptr;
}

RequestHandler MakeProxyHandler(http1::ClientTransportSource* clientTransportSource) {
    return
        [clientTransportSource](Request&& request, Responder&& responder) {
            new PerRequestProxyHandler(clientTransportSource, std::move(request), std::move(responder));
        };
}

} // namespace shitty
