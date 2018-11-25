#pragma once

#include <memory>

#include "RequestHandler.h"
#include "Transport.h"

namespace shitty {

class ServerTransport: virtual public Transport {
public:
    virtual void sendResponse(const Response&) = 0;
    virtual void onRequest(Request&&) = 0;

protected:
    void setGeneralHeaders(Headers&) override;

private:
    std::unique_ptr<RequestHandler> handler_;
};

}
