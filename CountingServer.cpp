#include <shitty/Server.h>
#include <shitty/CountingResponder.h>

using namespace shitty;

int main() {
    Server()
        .addHandler("/", CountingResponder())
        .run();
}
