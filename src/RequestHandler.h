#pragma once

#include <functional>

#include "Request.h"
#include "Responder.h"

namespace shitty {

using RequestHandler = std::function<void(Request&&, Responder&&)>;

} // namespace
