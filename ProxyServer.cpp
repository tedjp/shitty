#include <shitty/ClientTransportSource.h>
#include <shitty/ProxyHandler.h>
#include <shitty/Server.h>

using namespace shitty;

int main() {
    Server server;
    ClientTransportSource client_source(server.epollFD());

    server.addHandler("/", ProxyHandler(&client_source));
    server.run();
}
