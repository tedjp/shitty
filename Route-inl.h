namespace shitty {

Route::Route(const std::string& path):
    path_(path)
{}

Route::Route(std::string&& path):
    path_(std::move(path))
{}

inline const std::string& Route::path() const {
    return path_;
}

namespace detail {

// Class to make per-request proxy objects so that static handlers can be
// accessed as if they were per-request factory-served routes.
class StaticRouteProxy: public RequestHandler {
public:
    explicit StaticRouteProxy(StaticResponder* static_responder):
        static_responder_(static_responder)
    {}

    void onRequest(Request&& request, ServerTransport *transport) override {
        static_responder_->onRequest(std::move(request), transport);
    }

private:
    StaticResponder* static_responder_;
};

class StaticRouteHandlerFactory: public RequestHandlerFactory {
public:
    explicit StaticRouteHandlerFactory(StaticResponder* responder):
        responder_(responder)
    {}

    std::unique_ptr<RequestHandler> getHandler() const override {
        return std::make_unique<StaticRouteProxy>(responder_);
    }

private:
    StaticResponder *responder_;
};

} // namespace detail

StaticRoute::StaticRoute(const std::string& path, const StaticResponder& responder):
    Route(path),
    responder_(responder),
    factory_(std::make_unique<detail::StaticRouteHandlerFactory>(&responder_))
{}

} // namespace shitty
