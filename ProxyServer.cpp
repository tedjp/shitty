#include <shitty/ClientTransportSource.h>
#include <shitty/ProxyHandler.h>
#include <shitty/Server.h>

using namespace shitty;

int main() {
    Server server;
    ClientTransportSource client_source(server.epollFD());

#if 0
    server.addStaticHandler(
            "/version",
            {"Content-type: text/plain"},
            "Version 1.0");
#endif
    server.addStaticHandler<std::initializer_list<std::string>, std::string>(
            std::string("/version"),
            {std::string("Content-type: text/plain")},
            std::string("Version 1.0"));

#if 0
    server.addHandler("/",
            std::make_unique<ProxyHandlerFactory>(&client_source));
#endif

    //server.addHandler<ProxyHandler>("/", &client_source);
    server.run();
}
