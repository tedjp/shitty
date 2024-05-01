#pragma once

#include <memory>
#include <string>

#include "RequestHandlerFactory.h"
#include "StaticResponder.h"

namespace shitty { 

// A Route is a prefix path and a handler function.
class Route {
public:
    explicit Route(const std::string& path);
    explicit Route(std::string&& path);
    virtual ~Route() = default;

    inline const std::string& path() const;

    virtual std::unique_ptr<RequestHandler> getHandler() const = 0;

private:
    std::string path_;
};

namespace detail {
class StaticRouteHandlerFactory;
}

// A route that reuses a single handler function for all requests.
class StaticRoute: public Route {
public:
    StaticRoute(const std::string& path, std::unique_ptr<StaticResponder>&& responder);

    std::unique_ptr<RequestHandler> getHandler() const override;

private:
    std::unique_ptr<StaticResponder> responder_;

    std::unique_ptr<detail::StaticRouteHandlerFactory> factory_;
};

// Route whose handler function spawns a new handler instance for each request,
// which may be stateful.
class FactoryRoute: public Route {
public:
    FactoryRoute(const std::string& path, std::unique_ptr<RequestHandlerFactory>&& factory);

    std::unique_ptr<RequestHandler> getHandler() const override;

private:
    std::unique_ptr<RequestHandlerFactory> factory_;
};

} // namespace shitty

#include "Route-inl.h"
