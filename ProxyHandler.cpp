#include <iostream>

#include "ProxyHandler.h"

using namespace shitty;

ProxyHandler::ProxyHandler(ClientTransportSource* source):
    client_transport_source_(source)
{}

void ProxyHandler::handle(Request&& request, ServerTransport *transport) {
    front_transport_  = transport;
    sendBackendRequest(std::move(request));
}

void ProxyHandler::onRequest(Request&& request) {
    sendBackendRequest(std::move(request));
}

void ProxyHandler::sendBackendRequest(Request&& request) {
    acquireBackendTransport(request);
    backend_transport_->sendRequest(std::move(request));
    std::cerr << "backend request sent" << std::endl;
}

void ProxyHandler::respond(Response&& response) {
    std::cerr << "responding to frontend" << std::endl;
    front_transport_->sendResponse(std::move(response));
}

void ProxyHandler::onBackendResponse(Response&& response) {
    std::cerr << "backend response" << std::endl;
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
    std::cerr << "releasing backend connection to pool" << std::endl;
    backend_transport_->resetHandler();
    client_transport_source_->putTransport(std::move(backend_transport_));
    backend_transport_ = nullptr;
}
