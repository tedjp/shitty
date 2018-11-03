#include "static_responder.h"

using std::string;
using shitty::StaticResponder;

void StaticResponder::writeHeaders(std::ostream& dest) {
    for (const Header& header: response_.headers().kv_) {
        const std::string& name = header.first;
        const std::string& value = header.second;

        dest.write(name.data(), name.size());
        dest.write(": ", 2);
        dest.write(value.data(), value.size());
        dest.write("\r\n", 2);
    }
}

void StaticResponder::writeBody(std::ostream& dest) {
    const std::string& body = response_.body();
    dest.write(body.data(), body.size());
}
