#pragma once

#include "Message.h"

namespace shitty {

class Response {
public:
    // Constructors allow implicit conversions on purpose
    Response(std::string&& body);
    Response(std::initializer_list<std::string> headers, std::string&& body);
    Response(std::initializer_list<Header> headers, std::string&& body);
    Response(unsigned status_code = 200, std::string&& body = std::string());
    Response(unsigned status_code = 200, Message&& msg = Message());
    Message message;

    Headers& headers() { return message.headers(); }
    const Headers& headers() const { return message.headers(); }

    std::string& body() { return message.body(); }
    const std::string& body() const { return message.body(); }

    inline unsigned statusCode() const;
    inline void setStatusCode(unsigned status_code);

private:
    unsigned status_code_ = 200;
};

inline Response::Response(std::string&& body):
    message(std::move(body))
{}

inline Response::Response(unsigned status_code, std::string&& body):
    message(std::move(body)),
    status_code_(status_code)
{}

inline unsigned Response::statusCode() const {
    return status_code_;
}

inline void Response::setStatusCode(unsigned status_code) {
    status_code_ = status_code;
}

}
