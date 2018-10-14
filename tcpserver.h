#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include "socket.h"

namespace shitty {

class TCPServer {
public:
    class Handler {
    public:
        virtual void onAccept(Socket&& client) {
            client.close();
        }
    };

    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_SERVICE;

    TCPServer(Handler& handler, Socket&& socket);

    // Self-contained event loop
    void run();

protected:
    Socket socket_;
    Handler& handler_;
};

TCPServer
makeTCPServer(TCPServer::Handler& handler, uint16_t port);

TCPServer
makeTCPServer(
        TCPServer::Handler& handler,
        const std::string& host = TCPServer::DEFAULT_HOST,
        const std::string& service = TCPServer::DEFAULT_SERVICE);

} // namespace shitty
