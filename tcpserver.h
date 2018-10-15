#pragma once

#include <memory>
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

    TCPServer(std::unique_ptr<Handler>&& handler, Socket&& socket);

    // Self-contained event loop
    void run();

protected:
    Socket socket_;
    std::unique_ptr<Handler> handler_;
};

TCPServer
makeTCPServer(std::unique_ptr<TCPServer::Handler>&& handler, uint16_t port);

TCPServer
makeTCPServer(
        std::unique_ptr<TCPServer::Handler>&& handler,
        const std::string& host = TCPServer::DEFAULT_HOST,
        const std::string& service = TCPServer::DEFAULT_SERVICE);

} // namespace shitty
