#include <shitty/Server.h>
#include <shitty/Handler.h>

using namespace shitty;

class CountingResponder: public Handler {
public:
    void handle(Request&& request, Transport *transport) override {
        Response response(
                {"Content-type: text/plain"},
                std::to_string(++request_count_) + '\n');

        transport->writeResponse(response);
    }

private:
    uint64_t request_count_ = 0;
};

int main() {
    Server()
        .addHandler("/", CountingResponder())
        .run();
}
