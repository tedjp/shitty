#include <string>

#include <shitty/server.h>
#include <shitty/static_responder.h>

using namespace shitty;

int main() {
    Server()
        .addHandler("/", StaticResponder("Hello, world!\n"))
        .run();
}
