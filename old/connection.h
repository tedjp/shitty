#pragma once

#include <string>
#include <unordered_map>

#include "iobuf.h"
#include "settings.h"
#include "stream.h"
#include "socket.h"

namespace shitty::http2 {

// A connection awaiting the preface
class UnconfirmedConnection {
public:
    void onData();

private:
    Socket socket_;
};

class Connection {
public:
    enum class State {
        UNCONFIRMED,
        OK,
        BAD
    };

    // Construct from HTTP/1 Upgrade
    explicit Connection(Socket&& socket, const std::string& settings_b64):
        socket_(std::move(socket)),
        state_(State::OK) // too soon?
    {
        parseBase64Settings(settings_b64);
        constructInputBuffer();
    }

    // Construct native (non-upgraded)
    explicit Connection(Socket&& socket):
        socket_(std::move(socket))
    {
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = default;

    // For connecting to event loops:
    // fd to poll:
    int fd() {
        return socket_.getRawFD();
    }

    // callback when data is available on the fd
    void onData();

private:
    void constructInputBuffer() {
        input_ = IOBuf(settings_.get(Settings::MAX_FRAME_SIZE));
    }

    void parseBase64Settings(const std::string& b64);
    void parseSettings(IOBuf& raw_settings);
    Socket socket_;
    Settings settings_;
    IOBuf input_;
    std::unordered_map<stream_id_t, Stream> streams_;
    State state_ = State::UNCONFIRMED;
    // TODO: Set short timeout for unconfirmed connections
    // Also timeouts for idle
};
} // namespace shitty::http2
