#pragma once

#include "Message.h"

namespace shitty {

class Response {
public:
    // XXX: non-explicit?
    explicit Response(std::string&& body);
    Response(unsigned status_code = 200, std::string&& body = std::string());
    Message message;

    inline unsigned statusCode() const;
    inline void setStatusCode(unsigned status_code);

private:
    unsigned status_code_;
};

inline Response::Response(std::string&& body):
    message(std::move(body)),
    status_code_(200)
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
