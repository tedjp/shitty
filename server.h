#pragma once

#include <string>

#include "socket.h"

namespace shitty::http2 {

class Server {
public:
    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_SERVICE;

    Server(Socket&& socket);

    static Server Create(const std::string& host = DEFAULT_HOST, const std::string& port = DEFAULT_SERVICE);

    // Blocking self-contained event loop
    void run();

private:
    Socket socket_;
};

} // namespace shitty::http2
