#include <Server.h>
#include <StaticResponder.h>

using namespace shitty;

int main() {
    Server()
        .addStaticHandler("/", "Hello, world!\n")
        .run();
}
