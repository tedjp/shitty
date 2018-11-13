#include "CountingResponder.h"

using shitty::CountingResponder;

CountingResponder::CountingResponder():
    request_count_(0)
{}

CountingResponder::CountingResponder(CountingResponder&& other):
    request_count_(other.request_count_.load())
{}

void CountingResponder::handle(Request&& request, Transport *transport) {
    Response resp(std::to_string(++request_count_) + '\n');
    resp.message.headers().set("Content-Type", "text/plain; charset=us-ascii");
    transport->writeResponse(resp);
}
