#include <atomic>

#include "Handler.h"

namespace shitty {

class CountingResponder: public Handler {
public:
    CountingResponder();
    CountingResponder(CountingResponder&&);
    void handle(Request&& request, Transport *transport);

private:
    std::atomic_uint_fast64_t request_count_;
};

}
