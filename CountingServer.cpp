#include <shitty/Handler.h>
#include <shitty/Response.h>
#include <shitty/Server.h>

using namespace shitty;

class CountingResponder: public Handler {
public:
    void handle(Request&& request, ServerTransport *transport) override {
        Response response(
                {"Content-type: text/plain"},
                std::to_string(++request_count_) + '\n');

        transport->sendResponse(response);
    }

private:
    uint64_t request_count_ = 0;
};

int main() {
    Server()
        .addHandler("/", CountingResponder())
        .run();
}
