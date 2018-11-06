#pragma once

#include <string>

#include "Headers.h"
#include "Message.h"

namespace shitty {

class Request {
public:
    Request(
            const std::string& method,
            const std::string& path,
            Headers&& headers = Headers());
    // HTTP/2 version:
    //Request(Headers&& headers);

    inline const std::string&
        method() const;
    inline const std::string&
        path() const;
    inline Headers&
        headers();
    inline const Headers&
        headers() const;
    inline const std::string&
        body() const;
    inline std::string&
        body();


private:
    // Request-Line fields
    std::string method_;
    std::string path_;

    Message message_;
};

inline const std::string& Request::method() const {
    return method_;
}

inline const std::string& Request::path() const {
    return path_;
}

inline Headers& Request::headers() {
    return message_.headers();
}

inline const Headers& Request::headers() const {
    return message_.headers();
}

inline const std::string& Request::body() const {
    return message_.body();
}

inline std::string& Request::body() {
    return message_.body();
}

}
