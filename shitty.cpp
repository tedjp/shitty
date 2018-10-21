#include "server.h"

int main(void) {
    shitty::http2::Server().run();
    return 0;
}
