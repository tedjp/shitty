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
            Headers&& headers);
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

private:
    // Request-Line fields
    std::string method_;
    std::string path_;

    Message message_;
};

const std::string& Request::method() const {
    return method_;
}

const std::string& Request::path() const {
    return path_;
}

Headers& Request::headers() {
    return message_.headers();
}

const Headers& Request::headers() const {
    return message_.headers();
}

}
