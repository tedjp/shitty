#include <http1/ClientTransportSource.h>
#include <ProxyHandler.h>
#include <Server.h>

using namespace shitty;

int main() {
    Server server;
    http1::ClientTransportSource client_source(server.epollFD());

    server.addHandler("/", std::make_unique<ProxyHandlerFactory>(&client_source));
    server.run();
}
