#include <cerrno>
#include <cstring>
#include <string>

#include "Error.h"

namespace shitty {

std::runtime_error
error_errno(const char *prefix) {
    return error_errno(prefix, errno);
}

std::runtime_error
error_errno(const char *prefix, int err) {
    return std::runtime_error(std::string(prefix) + ": " + ::strerror(err));
}

} // namespace shitty
