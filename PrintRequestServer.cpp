#include <iostream>

#include "RequestHandler.h"
#include "Server.h"

using namespace shitty;

class PrintRequestHandler: public StaticResponder {
public:
    PrintRequestHandler():
        StaticResponder(200, Message())
    {}

    void onRequest(Request&& request, ServerStream *stream) override {
        using std::cout;

        cout << request.method() << ' ' << request.path() << '\n';
        for (const auto& header: request.headers().kv_)
            cout << header.first << ": " << header.second << '\n';
        cout << '\n';

        if (!request.body().empty()) {
            cout << request.body();
            cout << '\n';
        }

        cout.flush();

        StaticResponder::onRequest(std::move(request), stream);
    }
};

int main(void) {
    Server s;
    s.addRoute(std::make_unique<StaticRoute>("/", std::make_unique<PrintRequestHandler>()));
    s.run();
}
