#include <functional>

#include "tcpserver.h"

#include "connection.h"

using namespace shitty;

namespace shitty::http2 {

class ClientHandler {
};

class Server: public tcpserver::Server {
public:
    Server();

    void onNewConnection(Socket&& socket) override;

    // event loop
    void run();

private:
    std::unordered_map<Socket, ClientHandler> clients_;

    SafeFD epollfd_;
};

} // namespace shitty::http2
