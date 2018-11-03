#include "responder.h"

using shitty::Responder;

void Responder::writeResponse(std::ostream& dest) {
    writeHeaders(dest);
    dest.write("\r\n", 2);
    writeBody(dest);
}
