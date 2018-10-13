#include <cerrno>
#include <cstring>
#include <string>

#include "error.h"

namespace shitty {

std::runtime_error
error_errno(const char *prefix) {
    return std::runtime_error(std::string(prefix) + ": " + ::strerror(errno));
}

} // namespace shitty
