#include <cstring>
#include <limits>

// TODO: header cleanup
#include "HTTP1.h"
#include "Transport.h"
#include "../Payload.h"
#include "../Request.h"
#include "../StatusStrings.h"
#include "../StringUtils.h"

using namespace shitty::http1;
using shitty::Headers;
using shitty::Request;
using shitty::Response;
using shitty::StreamBuf;

Transport::Transport(Connection* connection):
    connection_(connection)
{}

void Transport::onInput(StreamBuf& input_buffer) {
    bool continu;
    do {
        continu = processInput(input_buffer);
    } while (continu && !input_buffer.empty());
}

// return code is whether we should try to process more input.
bool Transport::processInput(StreamBuf& input_buffer) {
    if (input_buffer.isEmpty())
        return false;

    if (incoming_message_
            && message_headers_complete_
            && !isMessageComplete())
    {
        readBody(input_buffer);
        return !isMessageComplete();
    }

    std::optional<std::string> oline = getLine(input_buffer);
    if (!oline.has_value())
        return false; // incomplete line

    std::string& line = *oline;

    if (!incoming_message_.has_value()) {
        message_headers_complete_ = false;
        incoming_message_ = IncomingMessage{std::move(line)};
        return true;
    }

    if (line.empty()) {
        markHeadersComplete();
        return !isMessageComplete();
    }

    if (line[0] == ' ' || line[0] == '\t') {
        headerContinuation(std::move(line));
        return true;
    }

    headerLine(std::move(line));
    return true;
}

void Transport::readBody(StreamBuf& input_buf) {
    if (expected_body_length_ == -1)
        readChunked(input_buf);
    else
        readContent(input_buf);
}

void Transport::readChunked(StreamBuf& input_buf) {
    // TODO
    throw std::runtime_error("Chunked input not yet supported");
}

void Transport::readContent(StreamBuf& input_buf) {
    size_t len = std::max(input_buf.size(), static_cast<size_t>(expected_body_length_));

    incoming_message_->message.body().append(std::string(input_buf.data(), len));
    input_buf.advance(len);

    if (expected_body_length_ == static_cast<ssize_t>(incoming_message_->message.body().size()))
        onEndOfMessage();

    if (!input_buf.empty()) {
        // more input (a pipelined request)
        onInput(input_buf);
    }
}

void Transport::onEndOfMessage() {
    handleMessage();
}

bool Transport::isMessageComplete() {
    return incoming_message_.has_value()
        && static_cast<ssize_t>(incoming_message_->message.body().size()) == expected_body_length_;
}

void Transport::markHeadersComplete() {
    message_headers_complete_ = true;

    Headers& headers = incoming_message_->message.headers();

    expected_body_length_ = getContentLength(headers);

    onEndOfMessageHeaders(headers);

    if (isMessageComplete()) {
        handleMessage();
        return;
    }

    // Wait for request body.
}

void Transport::handleMessage() {
    handleIncomingMessage(std::move(incoming_message_.value()));
    resetIncomingMessage();
}

void Transport::resetIncomingMessage() {
    incoming_message_.reset();
    last_header_.clear();
    message_headers_complete_ = false;
    expected_body_length_ = 0;
}

void Transport::headerLine(std::string&& line) {
    size_t colon = line.find(':');
    if (colon == line.npos)
        throw std::runtime_error("Malformed header: no colon");

    // Optimize for single space after colon
    size_t start_of_value = colon;
    if (colon < line.size() - 1 && line[colon + 1] == ' ')
        start_of_value += 1;

    std::string value = line.substr(start_of_value);

    // reuse line as header name
    std::string& name = line;
    name.resize(colon);
    // name is not allowed to have LWS padding so no need to trim it.
    // but must be normalized to lowercase for sanity
    asciiLower(name);

    trimLWS(value);

    last_header_ = name;
    incoming_message_->message.headers().add(std::move(name), std::move(value));
}

void Transport::headerContinuation(std::string&& line) {
    if (last_header_.empty())
        throw std::runtime_error("Erroneous header continuation");

    auto& header_map = incoming_message_->message.headers().kv_;
    auto it = header_map.find(last_header_);
    if (it == header_map.end())
        throw std::logic_error("Failed to find header for continuation");

    it->second.append(std::move(line));
}

void Transport::sendMessage(
        const std::string& first_line,
        const Message& message) {
    Payload payload(&connection_->outgoingStreamBuf());

    payload.send(first_line.data(), first_line.size());
    payload.send("\r\n", 2);

    Headers outgoingHeaders(message.headers());
    setGeneralHeaders(outgoingHeaders);
    setContentLength(outgoingHeaders, message.body().size());

    payload.send(renderHeaders(outgoingHeaders));

    // Combine header & body if there's room.
    if (message.body().size() < payload.tailroom()) {
        payload.send(message.body());
        connection_->send(reinterpret_cast<const char*>(payload.data()), payload.size());
    } else {
        payload.flush();
        connection_->send(message.body().data(), message.body().size());
    }
}

IncomingMessage
Transport::messageFromFirstLine(const std::string& first_line) {
    return {first_line};
}
