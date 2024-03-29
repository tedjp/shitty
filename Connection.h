#pragma once

#include <array>
#include <memory>

#include "ConnectionManager.h"
#include "EventReceiver.h"
#include "Payload.h"
#include "StreamBuf.h"
#include "Transport.h"

namespace shitty {

class Request;

class Connection: public EventReceiver {
public:
    // TODO: Make these static constructors for better naming, or inherit as
    // separate types.
    // Actually the differentiator isn't client/server but self-handle vs.
    // callback-handle...
    // Server-side (receive) connection
    //Connection(int epfd, int fd, ServerTransport::req_handler_t&& req_handler);
    // Client-side (send) connection
    //Connection(int epfd, int fd, ClientTransport::resp_handler_t&& resp_handler);
    Connection(int epfd, int fd);
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) = delete;
    ~Connection();

    int fd() const {
        return fd_;
    }

    int getPollFD() const override;
    void onPollIn() override;
    void onPollOut() override;

    void setTransport(std::unique_ptr<Transport>&&);
    void upgradeTransport(std::unique_ptr<Transport>&&, Request&& initialRequest);
    Transport* getTransport();

    void setConnectionManager(ConnectionManager *manager);

    // Send a buffer.
    //
    // Any data that cannot be sent immediately will be queued in the outgoing_
    // StreamBuf. Use a Payload to combine small writes into a single TCP packet.
    void send(const char *data, size_t len);
    void send(const Payload& payload);

    // Read from the StreamBuf
    ptrdiff_t recv(void *buf, size_t buflen);

    void close();

    bool isOpen() const;
    bool isClosed() const;

    inline bool operator==(const Connection& other);

    Payload getOutgoingPayload();

private:
    void subscribe_to_input();
    void updateSubscription();

    void send_immediate(const char *data, size_t len);
    void queue_and_send(const char *data, size_t len);

    // client fd.
    int fd_ = -1;
    // epoll event fd (not owned).
    int epfd_ = -1;
    StreamBuf incoming_, outgoing_;
    std::unique_ptr<Transport> transport_;
    ConnectionManager *manager_;
};

inline Transport* Connection::getTransport() {
    return transport_.get();
}

inline bool Connection::isOpen() const {
    return fd_ != -1;
}

inline bool Connection::isClosed() const {
    return fd_ == -1;
}

inline bool Connection::operator==(const Connection& other) {
    return fd_ == other.fd_;
}

} // namespace shitty

template <>
struct std::hash<shitty::Connection> {
    size_t operator()(const shitty::Connection& c) const {
        return static_cast<size_t>(c.fd());
    }
};
