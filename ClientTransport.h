#pragma once

#include <functional>

#include "Request.h"
#include "Transport.h"

namespace shitty {

class ClientTransport: virtual public Transport {
public:
    using resp_handler_t = std::function<void(Response&&, ClientTransport*)>;

    virtual void sendRequest(const Request&) = 0;
    virtual void setHandler(resp_handler_t&&) = 0;
    virtual void resetHandler() = 0;

protected:
    void setGeneralHeaders(Headers&) override;
};

}
