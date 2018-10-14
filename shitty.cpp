#include "server.h"

int main(void) {
    auto s = shitty::http2::Server::Create();
    s.run();

    return 0;
}
