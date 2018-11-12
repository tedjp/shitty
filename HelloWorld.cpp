#include <shitty/Server.h>
#include <shitty/StaticResponder.h>

using namespace shitty;

int main() {
    Server()
        .addHandler("/", StaticResponder("Hello, world!\n"))
        .run();
}
