#include "Response.h"
#include "Server.h"

using namespace shitty;

int main() {
    Server()
        .addRoute("/", Response("Hello, world!\n"))
        .run();
}
