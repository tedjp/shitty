#pragma once

#include <string>

#include "headers.h"

namespace shitty {

class Response {
public:
    virtual ~Response();
    virtual const std::string& body() const = 0;
    virtual const Headers& headers() const = 0;
};

} // namespace shitty
