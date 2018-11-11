#pragma once

namespace shitty {

class ConnectionManager {
public:
    ConnectionManager() = default;
    ConnectionManager(ConnectionManager&&) = default;
    ConnectionManager& operator=(ConnectionManager&&) = default;
    virtual ~ConnectionManager();

    virtual void removeConnection(int fd) = 0;
};

}
