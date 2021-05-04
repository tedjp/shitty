#pragma once

#include <utility>

#include "../Connection.h"
#include "../Request.h"
#include "../StreamBuf.h"
#include "../Transport.h"
#include "IncomingMessage.h"

namespace shitty::http1 {

class Transport: public shitty::Transport {
public:
    Transport(Connection* connection);

    // shitty::Transport overrides
    void onInput(StreamBuf& input_buffer) override;

    Connection* getConnection();

protected:
    virtual void handleIncomingMessage(IncomingMessage&&) = 0;
    virtual void sendMessage(const std::string& first_line, const Message& message);
    virtual void onEndOfMessageHeaders(Headers& headers) {}

private:
    static IncomingMessage
        messageFromFirstLine(const std::string& first_line);

    // TODO: Stop doing std::string ops on headers; just clone the entire header
    // blob and use std::string_view to point into it. Only headers with
    // continuation lines would need dynamic allocations. Then create a superset
    // string_or_view class that seamlessly owns & wraps either one.
    bool isMessageComplete();
    void markHeadersComplete();
    void onEndOfMessage();
    void headerContinuation(std::string&& line);
    void headerLine(std::string&& line);
    void readBody(StreamBuf& input);
    void readChunked(StreamBuf& input);
    void readContent(StreamBuf& input);
    void handleMessage();
    void resetIncomingMessage();
    bool processInput(StreamBuf& input);

    std::optional<IncomingMessage> incoming_message_;
    std::string last_header_; // used for header line continuations. empty == none
    bool message_headers_complete_ = false;
    // 0: no body. -1: chunked: >0: Content-Length
    // Chunked reader should simply reset it to the actual body length when the
    // terminal chunk is read.
    ssize_t expected_body_length_ = 0;

    Connection* connection_;
};

inline Connection* Transport::getConnection() {
    return connection_;
}

}
