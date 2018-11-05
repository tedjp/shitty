#pragma once

#include <memory>

#include "RequestRouter.h"
#include "StreamBuf.h"
#include "Transport.h"

namespace shitty {

class Connection {
public:
    // XXX: Passing the RequestRouter is kind of crap, but the Transport needs
    // to know where to send requests. Maybe split int a Connection base class
    // and HTTPConnection subclass.
    Connection(int epfd, int fd, RequestRouter *request_router);
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept;
    ~Connection();

    int fd() const {
        return fd_;
    }

    void onPollIn();
    void onPollOut();

    // Send a buffer.
    // Any data that cannot be sent immediately will be queued in the outgoing_
    // StreamBuf.
    void send(const void *data, size_t len);
    // Read from the StreamBuf
    ssize_t recv(void *buf, size_t buflen);

    //void close();

    inline bool operator==(const Connection& other);

private:
    // client fd.
    int fd_ = -1;
    // epoll event fd (not owned).
    int epfd_ = -1;
    StreamBuf incoming_, outgoing_;
    RequestRouter *request_router_;
    std::unique_ptr<Transport> transport_;
};

bool Connection::operator==(const Connection& other) {
    return fd_ == other.fd_;
}

}

template <>
struct std::hash<shitty::Connection> {
    size_t operator()(const shitty::Connection& c) const {
        return static_cast<size_t>(c.fd());
    }
};
