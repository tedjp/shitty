#include <shitty/http1/ClientTransportSource.h>
#include <shitty/ProxyHandler.h>
#include <shitty/Server.h>

using namespace shitty;

int main() {
    Server server;
    http1::ClientTransportSource client_source(server.epollFD());

    server.addHandler("/", std::make_unique<ProxyHandlerFactory>(&client_source));
    server.run();
}
