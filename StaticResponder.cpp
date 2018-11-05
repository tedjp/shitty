#include "StaticResponder.h"

using shitty::StaticResponder;

void StaticResponder::handle(Request&& req, Transport *transport) {
    transport->writeResponse(response_);
}
