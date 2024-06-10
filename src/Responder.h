#pragma once

#include "Response.h"
#include "ServerStream.h"

namespace shitty {

// Interface provided to a request handler for writing its response that only exposes as much of a
// ServerStream as necessary.
class Responder {
public:
    explicit Responder(ServerStream& serverStream);
    Responder(const Responder&) = delete;
    Responder(Responder&&) = default;
    Responder& operator=(const Responder&) = delete;
    Responder& operator=(Responder&&) = default;
    virtual ~Responder() = default;

    void respond(const Response& response);

private:
    // Never nullptr
    ServerStream* serverStream_ = nullptr;
};

inline Responder::Responder(ServerStream& serverStream):
    serverStream_(&serverStream)
{}

inline void Responder::respond(const Response& response) {
    serverStream_->sendResponse(response);
}

} // namespace shitty