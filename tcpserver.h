#pragma once

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "socket.h"

namespace shitty::tcpserver {

class Server {
public:
    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_SERVICE;

    Server(uint16_t port);

    Server(
            const std::string& host = Server::DEFAULT_HOST,
            const std::string& service = Server::DEFAULT_SERVICE);

    // Self-contained event loop
    void run();

    // Or: Run your own loop with this FD & callback
    int getFD();
    void onEvent();

protected:
    // Override this to do something with a new connection
    virtual void onNewConnection(Socket&& client) {
        client.close();
    }

    Socket socket_;
};

} // namespace shitty
