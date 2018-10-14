#pragma once

#include <set>
#include <string>

#include "socket.h"

namespace shitty {

class Handler {
public:
    virtual int getFD() = 0; // for epoll

};

template <typename HandlerType>
class TCPServer {
public:
    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_SERVICE;

    TCPServer(Socket&& socket);
    //TCPServer(const std::string& host = DEFAULT_HOST, const std::string& service = DEFAULT_SERVICE);

    static TCPServer<HandlerType> Create(const std::string& host = DEFAULT_HOST, const std::string& port = DEFAULT_SERVICE);

    // Self-contained event loop
    void run();

protected:
    Socket socket_;

private:
    std::unordered_map<int, HandlerType> clients_;
};

} // namespace shitty

#include "tcpserver-inl.h"
