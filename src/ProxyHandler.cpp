#include "ProxyHandler.h"
#include "http1/ServerTransport.h"

using namespace shitty;

ProxyHandler::ProxyHandler(http1::ClientTransportSource* source):
    client_transport_source_(source)
{
}

void ProxyHandler::onRequest(Request&& request, ServerStream* stream) {
    front_stream_  = stream;
    sendBackendRequest(std::move(request));
}

void ProxyHandler::sendBackendRequest(Request&& request) {
    acquireBackendTransport(request);
    backend_transport_->sendRequest(std::move(request));
}

void ProxyHandler::respond(Response&& response) {
    front_stream_->sendResponse(std::move(response));
}

void ProxyHandler::onBackendResponse(Response&& response) {
    releaseBackendTransport();
    respond(std::move(response));
}

void
ProxyHandler::acquireBackendTransport(const Request& request) {
    backend_transport_ = client_transport_source_->getTransport(
            std::bind(std::mem_fn(&ProxyHandler::onBackendResponse), this, std::placeholders::_1));
}

void
ProxyHandler::releaseBackendTransport() {
    backend_transport_->resetHandler();
    client_transport_source_->putTransport(std::move(backend_transport_));
    backend_transport_ = nullptr;
}
