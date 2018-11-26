#pragma once

#include <memory>
#include <string>

#include "RequestHandlerFactory.h"
#include "StaticResponder.h"

namespace shitty { 

class Route {
public:
    explicit Route(const std::string& path);
    explicit Route(std::string&& path);

    inline const std::string& path() const;

    virtual std::unique_ptr<RequestHandler> getHandler() const = 0;

private:
    std::string path_;
};

namespace detail {
class StaticRouteHandlerFactory;
}

class StaticRoute: public Route {
public:
    StaticRoute(const std::string& path, const StaticResponder& responder);
    // For non-copyable static responder
    StaticRoute(std::string&& path, StaticResponder&& responder);

    std::unique_ptr<RequestHandler> getHandler() const override;

private:
    StaticResponder responder_;

    std::unique_ptr<detail::StaticRouteHandlerFactory> factory_;
};

class FactoryRoute: public Route {
public:
    FactoryRoute(const std::string& path, std::unique_ptr<RequestHandlerFactory>&& factory);
    FactoryRoute(const std::string& path, const RequestHandlerFactory& factory);

    std::unique_ptr<RequestHandler> getHandler() const override;

private:
    std::unique_ptr<RequestHandlerFactory> factory_;
};

} // namespace shitty

#include "Route-inl.h"
