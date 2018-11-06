#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>

#include "HTTP1Transport.h"

#include "Request.h"

using shitty::Headers;
using shitty::HTTP1Transport;
using shitty::Request;

const char* HTTP1Transport::findEndOfLine(const char *buf, size_t len) {
    if (len == 0)
        return nullptr;

    const char *last_nl = nullptr;

    // Optimize for \n-terminated input
    // XXX: Is this faster than just calling memrchr?
    if (buf[len - 1] == '\n')
        last_nl = &buf[len - 1];
    else
        last_nl = reinterpret_cast<const char*>(memrchr(buf, '\n', len));

    if (last_nl == nullptr)
        return nullptr;

    // It's now safe to do a rawmemchr() up to last_nl
    // In addition, we *know* rawmemchr() will succeed.
    return reinterpret_cast<const char*>(rawmemchr(buf, '\n')) + 1;
}

void HTTP1Transport::onInput(StreamBuf& input_buffer) {
    bool continu;
    do {
        continu = processInput(input_buffer);
    } while (continu && !input_buffer.empty());
}

// return code is whether we should try to process more input.
bool HTTP1Transport::processInput(StreamBuf& input_buffer) {
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

void HTTP1Transport::readBody(StreamBuf& input_buf) {
    if (expected_body_length_ == -1)
        readChunked(input_buf);
    else
        readContent(input_buf);
}

void HTTP1Transport::readChunked(StreamBuf& input_buf) {
    // TODO
    throw std::runtime_error("Chunked input not yet supported");
}

void HTTP1Transport::readContent(StreamBuf& input_buf) {
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

void HTTP1Transport::onEndOfRequest() {
    handleRequest();
}

bool HTTP1Transport::isRequestComplete() {
    return request_in_progress_.has_value()
        && static_cast<ssize_t>(request_in_progress_->body().size()) == expected_body_length_;
}

// Get content-length indication, which may be -1 if chunked transfer-encoding
// is indicated.
static ssize_t get_content_length(const Headers& headers) {
    const auto& header_map = headers.kv_;

    auto content_length_it = header_map.find("content-length");
    if (content_length_it != header_map.end()) {
        size_t len;

        try {
            len = std::stoul(content_length_it->second);
        } catch (std::logic_error&) {
            throw std::runtime_error("Invalid Content-Length header");
        }

        if (len > static_cast<size_t>(std::numeric_limits<ssize_t>::max()))
            throw std::runtime_error("Content-Length too big");

        return static_cast<ssize_t>(len);
    }

    auto transfer_encoding_it = header_map.find("transfer-encoding");
    if (transfer_encoding_it != header_map.end())
        return -1;

    // No indication of a body
    return 0;
}

void HTTP1Transport::onEndOfRequestHeaders() {
    request_headers_complete_ = true;

    expected_body_length_ = get_content_length(request_in_progress_->headers());

    if (isRequestComplete()) {
        handleRequest();
        return;
    }

    // Wait for request body.
}

void HTTP1Transport::handleRequest() {
    request_router_->route(std::move(*request_in_progress_), this);
    resetIncomingRequest();
}

void HTTP1Transport::resetIncomingRequest() {
    request_in_progress_.reset();
    last_header_.clear();
    request_headers_complete_ = false;
    expected_body_length_ = 0;
}

static void trimTrailingLWS(std::string& s) {
    size_t initial_size = s.size();
    size_t i = initial_size;
    while (i != 0) {
        --i;

        if (s[i] == ' ' || s[i] == '\t')
            continue;

        break;
    }

    size_t new_size = i + 1;
    if (new_size != initial_size)
        s.resize(i);
}

static void trimLeadingLWS(std::string& s) {
    size_t leading_whitespace = 0;
    size_t slen = s.size();
    for (size_t i = 0; i < slen; ++i) {
        if (s[i] == ' ' || s[i] == '\t')
            ++leading_whitespace;
    }

    if (leading_whitespace == 0)
        return;

    size_t new_size = slen - leading_whitespace;
    memmove(s.data(), s.data() + leading_whitespace, new_size);
    s.resize(new_size);
}

static void trimLWS(std::string& s) {
    trimTrailingLWS(s);
    trimLeadingLWS(s);
}

static void asciiLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](char c) {
            return static_cast<char>(::tolower(c));
        });
}

void HTTP1Transport::headerLine(std::string&& line) {
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

void HTTP1Transport::headerContinuation(std::string&& line) {
    if (last_header_.empty())
        throw std::runtime_error("Erroneous header continuation");

    auto& header_map = request_in_progress_->headers().kv_;
    auto it = header_map.find(last_header_);
    if (it == header_map.end())
        throw std::logic_error("Failed to find header for continuation");

    it->second.append(std::move(line));
}

std::string HTTP1Transport::renderHeaders(const Headers& headers) {
    std::string s;

    for (const auto& header: headers.kv_) {
        s.append(header.first);
        s.append(": ");
        s.append(header.second);
        s.append("\r\n");
    }

    s.append("\r\n");

    return s;
}

void HTTP1Transport::writeResponse(const Response& resp) {
    Connection* conn = connection_; // for convenience
    // XXX: A better API for building up a response buffer and sending it, eg.
    // create a StreamBuf, write into it, then send the entire StreamBuf to
    // Connection::send().
    // This is very TCP-inefficient and would benefit from that, or TCP_CORK as
    // a hack/workaround.
    std::string status_line = statusLine(resp);
    conn->send(status_line.data(), status_line.size());
    conn->send("\r\n", 2);
    // FIXME: Set Content-Length or Content-Encoding instead of trusting the
    // caller.
    std::string h1headers = renderHeaders(resp.message.headers());
    conn->send(h1headers.data(), h1headers.size());
    conn->send(resp.message.body().data(), resp.message.body().size());
}

std::string
HTTP1Transport::requestLine(const Request& req) {
    return req.method() + ' ' + req.path() + " HTTP/1.1";
}

std::string
HTTP1Transport::statusLine(const Response& resp) {
    // The majority of UTF-8 text is technically valid, except those that
    // contain \x00-x1f or \x7f.
    return "HTTP/1.1 " + std::to_string(resp.statusCode()) + " \xf0\x9f\x98\x8e";
}

Request
HTTP1Transport::requestFromRequestLine(const std::string& request_line) {
    size_t sp1 = request_line.find(' ');
    if (sp1 == request_line.npos || sp1 == 0)
        throw std::runtime_error("Malformed request");
    std::string method = request_line.substr(0, sp1);
    size_t sp2 = request_line.find(sp1 + 1, ' ');
    std::string path = request_line.substr(sp1 + 1, sp2 - sp1);
    // Ignoring HTTP-Version token
    return Request(std::move(method), std::move(path));
}

static void removeTrailingChar(std::string& s, char c) {
    size_t len = s.size();
    if (len != 0 && s[len - 1] == c)
        s.resize(len - 1);
}

static void trimNewline(std::string& s) {
    removeTrailingChar(s, '\n');
    removeTrailingChar(s, '\r');
}

std::optional<std::string>
HTTP1Transport::getLine(StreamBuf& buf) {
    const char *end_of_line = findEndOfLine(buf.data(), buf.size());
    if (end_of_line == nullptr)
        return std::nullopt;

    size_t line_len = end_of_line - buf.data();

    std::string line(buf.data(), line_len);

    buf.advance(line_len);

    trimNewline(line);

    return line;
}
