#include <shitty/PerRequestHandler.h>
#include <shitty/Response.h>
#include <shitty/Server.h>
#include <shitty/ServerTransport.h>

using namespace shitty;

class CountingResponder: public RequestHandler {
public:
    void onRequest(Request&& request, ServerTransport *transport) override {
        Response response(
                {"Content-type: text/plain"},
                std::to_string(++request_count_) + '\n');

        transport->sendResponse(response);
    }

private:
    uint64_t request_count_ = 0;
};

// This example illustrates how the PerRequestHandler creates a new instance of
// CountingResponder for each request, so every request gets the same response;
// that it is the first to be handled.
int main() {
    Server()
        .addHandler("/", PerRequestHandler<CountingResponder>())
        .run();
}
