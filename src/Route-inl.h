namespace shitty {

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

    void onRequest(Request&& request, ServerStream *stream) override {
        static_responder_->onRequest(std::move(request), stream);
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
} // namespace shitty
