#pragma once

#include <string>

#include "Headers.h"

namespace shitty {

class Message {
public:
    Message() = default;
    explicit Message(Headers&& headers);
    explicit Message(const std::string& body);
    explicit Message(std::string&& body);
    Message(const std::string& body, const std::pair<std::string, std::string> header...);
    Message(const std::string& body, const Headers& headers);
    ~Message();

    inline std::string& body();
    inline const std::string& body() const;
    inline Headers& headers();
    inline const Headers& headers() const;

private:
    std::string body_;
    Headers headers_;
};

inline const std::string&
Message::body() const {
    return body_;
}

inline std::string&
Message::body() {
    return body_;
}

inline const Headers&
Message::headers() const {
    return headers_;
}

inline Headers&
Message::headers() {
    return headers_;
}

} // namespace shitty
