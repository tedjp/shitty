#include "UnhandledRequestHandler.h"

static const shitty::Response unhandled_request_response(400, "No handler for request");

namespace shitty {

StaticResponder unhandled_request_handler = StaticResponder(unhandled_request_response);

}
