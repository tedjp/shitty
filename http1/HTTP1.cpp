#include <cstring>
#include <limits>
#include <string_view>

#include "../Error.h"
#include "../StatusStrings.h"
#include "HTTP1.h"

namespace shitty::http1 {

static const char*
findEndOfLine(const char *buf, size_t len) {
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

// Get content-length indication, which may be -1 if chunked transfer-encoding
// is indicated.
ssize_t getContentLength(const Headers& headers) {
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

std::string
renderHeaders(const Headers& headers) {
    static const std::string CRLF{"\r\n"};
    std::string s;

    for (const auto& header: headers.kv_) {
        s.append(header.first);
        s.append(": ");
        s.append(header.second);
        s.append(CRLF);
    }

    s.append(CRLF);

    return s;
}

std::string
requestLine(const Request& req) {
    return req.method() + ' ' + req.path() + " HTTP/1.1";
}

std::string
statusLine(const Response& resp) {
    // The majority of UTF-8 text is technically valid, except those that
    // contain \x00-x1f or \x7f, so for now I'm having some fun with the status
    // reason phrase (which is not carried in HTTP/2 at all).
    std::string status("HTTP/1.1 xxx \xf0\x9f\x98\x8e");
    const char *str = status_strings[resp.statusCode()];
    status[ 9] = str[0];
    status[10] = str[1];
    status[11] = str[2];
    return status;
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
getLine(StreamBuf& buf) {
    const char *end_of_line = findEndOfLine(buf.data(), buf.size());
    if (end_of_line == nullptr)
        return std::nullopt;

    size_t line_len = end_of_line - buf.data();

    std::string line(buf.data(), line_len);

    buf.advance(line_len);

    trimNewline(line);

    return line;
}

static std::string::size_type
findNextSpaceOrThrow(const std::string& s, std::string::size_type start = 0) {
    auto sp = s.find(' ', start);
    if (sp == s.npos)
        throw ProtocolError("Expected space character");
    return sp;
}

RequestLine
parseRequestLine(const std::string& request_line) {
    std::string::size_type sp1, sp2;

    try {
        sp1 = findNextSpaceOrThrow(request_line);
        sp2 = findNextSpaceOrThrow(request_line, sp1 + 1);
    }
    catch (const std::exception&) {
        throw ProtocolError("Malformed request line");
    }

    return RequestLine{
        request_line.substr(0, sp1),
        request_line.substr(sp1 + 1, sp2 - sp1 - 1),
        request_line.substr(sp2 + 1),
    };
}

template <typename NumericType>
static NumericType decimalValue(char c) __attribute__((const));
template <typename NumericType>
static NumericType decimalValue(char c) {
    if (!::isdigit(c))
        throw std::invalid_argument("non-decimal digit");

    return c - '0';
}

template <typename NumericType>
static NumericType codeToNumber(const char code[3]) {
    return
        decimalValue<NumericType>(code[0]) * 100 +
        decimalValue<NumericType>(code[1]) *  10 +
        decimalValue<NumericType>(code[2]);
}

StatusLine
parseStatusLine(const std::string& status_line) {
    std::string::size_type sp1, sp2;

    try {
        sp1 = findNextSpaceOrThrow(status_line);
        sp2 = findNextSpaceOrThrow(status_line, sp1 + 1);
    }
    catch (const std::exception&) {
        throw ProtocolError("Malformed status line");
    }

    std::string_view version(&status_line[0], sp1);
    std::string_view code(&status_line[sp1 + 1], sp2 - sp1 - 1);
    std::string_view reason_phrase(&status_line[sp2 + 1], status_line.size() - sp2 - 1);

    if (code.size() != 3)
        throw ProtocolError(std::string("Invalid status code: ") + std::string(code));

    uint_fast16_t numeric_code;

    try {
        numeric_code = codeToNumber<uint_fast16_t>(code.data());
    }
    catch (const std::exception&) {
        throw ProtocolError(std::string("Invalid status code: ") + std::string(code));
    }

    return {numeric_code, std::string(reason_phrase), std::string(version)};
}

} // namespace
