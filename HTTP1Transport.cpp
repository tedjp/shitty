#include <cstring>

#include "HTTP1Transport.h"

#include "Request.h"

using shitty::Headers;
using shitty::HTTP1Transport;
using shitty::Request;

char* HTTP1Transport::findEndOfHeader(char *buf, size_t len) {
    // TODO: There might be a slight performance benefit to check the last octet
    // manually before calling memrchr
    char *last_nl = reinterpret_cast<char*>(memrchr(buf, '\n', len));
    if (last_nl == nullptr)
        return nullptr;

    // It's now safe to use rawmemchr() up to last_nl

    // Linear search forward to the first "\n\r\n" (or "\n\n" to be permissive).
    char *nl = nullptr;
    for (
            char *start = buf;
            (nl = reinterpret_cast<char*>(rawmemchr(start, '\n'))) != last_nl;
            start = nl + 1)
    {
        if (nl[1] == '\n')
            return nl + 2;

        if (last_nl - nl >= 2) {
            if (nl[1] == '\r' && nl[2] == '\n')
                return nl + 3;
        }
    }

    return nullptr;
}

void HTTP1Transport::onInput(StreamBuf& input_buffer) {
    // TODO: Rewrite this to read a header line at a time to avoid iterating
    // over the same incomplete buffer every time.
    // Also enforce a max headers size.

#if 0 // TODO!
    char *end_of_header = findEndOfHeader(input_buffer.data(), input_buffer.size());
    if (end_of_header == nullptr)
        return; // Nothing to do yet.

    size_t header_len = end_of_header - input_buffer.data();

    Headers headers = parseHeaders(
            input_buffer.data(),
            header_len);

    input_buffer.advance(header_len);
#else
    Headers headers;
#endif

    // FIXME: Need to provide the Request-Line separately from the headers.
    Request request("GET", "/", std::move(headers));

    request_router_->route(std::move(request), this);
}

Headers HTTP1Transport::parseHeaders(const char *data, size_t len) {
    // TODO
    return Headers();
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
