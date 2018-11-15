#include <cstring>
#include <limits>

// TODO: header cleanup
#include "HTTP1.h"
#include "ServerTransport.h"
#include "../Payload.h"
#include "../Request.h"
#include "../StatusStrings.h"
#include "../StringUtils.h"

using namespace shitty::http1;
using shitty::Headers;
using shitty::Request;
using shitty::Response;
using shitty::StreamBuf;

void ServerTransport::onInput(StreamBuf& input_buffer) {
    bool continu;
    do {
        continu = processInput(input_buffer);
    } while (continu && !input_buffer.empty());
}

// return code is whether we should try to process more input.
bool ServerTransport::processInput(StreamBuf& input_buffer) {
    if (input_buffer.isEmpty())
        return false;

    if (request_in_progress_
            && request_headers_complete_
            && !isRequestComplete())
    {
        readBody(input_buffer);
        return !isRequestComplete();
    }

    std::optional<std::string> oline = getLine(input_buffer);
    if (!oline.has_value())
        return false; // incomplete line

    std::string& line = *oline;

    if (!request_in_progress_.has_value()) {
        request_headers_complete_ = false;
        request_in_progress_ = requestFromRequestLine(std::move(line));
        return true; // always try to read headers
    }

    if (line.empty()) {
        onEndOfRequestHeaders();
        return !isRequestComplete();
    }

    if (line[0] == ' ' || line[0] == '\t') {
        headerContinuation(std::move(line));
        return true;
    }

    headerLine(std::move(line));
    return true;
}

void ServerTransport::readBody(StreamBuf& input_buf) {
    if (expected_body_length_ == -1)
        readChunked(input_buf);
    else
        readContent(input_buf);
}

void ServerTransport::readChunked(StreamBuf& input_buf) {
    // TODO
    throw std::runtime_error("Chunked input not yet supported");
}

void ServerTransport::readContent(StreamBuf& input_buf) {
    size_t len = std::max(input_buf.size(), static_cast<size_t>(expected_body_length_));

    request_in_progress_->body().append(std::string(input_buf.data(), len));
    input_buf.advance(len);

    if (expected_body_length_ == static_cast<ssize_t>(request_in_progress_->body().size()))
        onEndOfRequest();

    if (!input_buf.empty()) {
        // more input (a pipelined request)
        onInput(input_buf);
    }
}

void ServerTransport::onEndOfRequest() {
    handleRequest();
}

bool ServerTransport::isRequestComplete() {
    return request_in_progress_.has_value()
        && static_cast<ssize_t>(request_in_progress_->body().size()) == expected_body_length_;
}

void ServerTransport::onEndOfRequestHeaders() {
    request_headers_complete_ = true;

    expected_body_length_ = getContentLength(request_in_progress_->headers());

    if (isRequestComplete()) {
        handleRequest();
        return;
    }

    // Wait for request body.
}

void ServerTransport::handleRequest() {
    request_router_->route(std::move(*request_in_progress_), this);
    resetIncomingRequest();
}

void ServerTransport::resetIncomingRequest() {
    request_in_progress_.reset();
    last_header_.clear();
    request_headers_complete_ = false;
    expected_body_length_ = 0;
}

void ServerTransport::headerLine(std::string&& line) {
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
    request_in_progress_->headers().add(std::move(name), std::move(value));
}

void ServerTransport::headerContinuation(std::string&& line) {
    if (last_header_.empty())
        throw std::runtime_error("Erroneous header continuation");

    auto& header_map = request_in_progress_->headers().kv_;
    auto it = header_map.find(last_header_);
    if (it == header_map.end())
        throw std::logic_error("Failed to find header for continuation");

    it->second.append(std::move(line));
}

void ServerTransport::writeResponse(const Response& resp) {
    Payload payload(&connection_->outgoingStreamBuf());

    std::string status_line = statusLine(resp);

    payload.send(status_line.data(), status_line.size());
    payload.send("\r\n", 2);

    Headers realHeaders(resp.message.headers());
    setResponseHeaders(realHeaders);
    setContentLength(realHeaders, resp.message.body().size());

    payload.send(renderHeaders(realHeaders));

    // Combine header & body if there's room.
    if (resp.message.body().size() < payload.tailroom()) {
        payload.send(resp.message.body());
        // XXX: This API is grubby
        connection_->send(reinterpret_cast<const char*>(payload.data()), payload.size());
    } else {
        payload.flush();
        connection_->send(resp.message.body().data(), resp.message.body().size());
    }
}

void ServerTransport::setResponseHeaders(Headers& headers) {
    setStandardHeaders(headers);
}

Request
ServerTransport::requestFromRequestLine(const std::string& request_line) {
    size_t sp1 = request_line.find(' ');
    if (sp1 == request_line.npos || sp1 == 0)
        throw std::runtime_error("Malformed request");
    std::string method = request_line.substr(0, sp1);
    size_t sp2 = request_line.find(sp1 + 1, ' ');
    std::string path = request_line.substr(sp1 + 1, sp2 - sp1);
    // Ignoring HTTP-Version token
    return Request(std::move(method), std::move(path));
}
