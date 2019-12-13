#include "Route.h"

using namespace shitty;

Route::Route(const std::string& path):
    path_(path)
{}

Route::Route(std::string&& path):
    path_(std::move(path))
{}

StaticRoute::StaticRoute(const std::string& path, std::unique_ptr<StaticResponder>&& responder):
    Route(path),
    responder_(move(responder)),
    factory_(std::make_unique<detail::StaticRouteHandlerFactory>(responder_.get()))
{}

std::unique_ptr<RequestHandler>
StaticRoute::getHandler() const {
    return factory_->getHandler();
}

FactoryRoute::FactoryRoute(const std::string& path, std::unique_ptr<RequestHandlerFactory>&& factory):
    Route(path),
    factory_(std::move(factory))
{}

std::unique_ptr<RequestHandler>
FactoryRoute::getHandler() const {
    return factory_->getHandler();
}
