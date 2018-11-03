#pragma once

#include <string>

#include "response.h"

namespace shitty {

class StaticResponse: public Response {
public:
    // Intentionally implicit
    StaticResponse(const std::string& body);
    StaticResponse(const std::string& body, const std::pair<std::string, std::string> header...);
    StaticResponse(const std::string& body, const Headers& headers);

    inline const std::string&
        body() const override;
    inline const Headers&
        headers() const override;

private:
    std::string body_;
    Headers headers_;
};

const std::string&
StaticResponse::body() const {
    return body_;
}

const Headers&
StaticResponse::headers() const {
    return headers_;
}

} // namespace shitty
