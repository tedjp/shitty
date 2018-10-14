#pragma once

#include <unordered_map>

#include "connection.h"
#include "tcpserver.h"

namespace shitty::http2 {
class Server: public TCPServer<Connection> {
};
} // namespace shitty::http2
