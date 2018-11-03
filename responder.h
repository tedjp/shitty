#pragma once

#include <ostream>
#include <string>

namespace shitty {

class Responder {
public:
    virtual void writeHeaders(std::ostream& dest) = 0;
    virtual void writeBody(std::ostream& dest) = 0;
    virtual void writeResponse(std::ostream& dest);
};

} // namespace
