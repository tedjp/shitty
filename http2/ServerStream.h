#pragma once

#include "../ServerStream.h"

namespace shitty::http2 {

class ServerStream: public shitty::ServerStream {
public:
    void onRequest(Request&&) override;
    void sendResponse(const Response&) override;
};

} // namespace
