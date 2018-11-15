#pragma once

#include <functional>
#include <utility>

#include "../Connection.h"
#include "../Headers.h"
#include "../Request.h"
#include "../RequestRouter.h"
#include "../StreamBuf.h"
#include "../Transport.h"

namespace shitty::http1 {

class ClientTransport: public shitty::ClientTransport {
public:
    using resp_handler_t = std::function<void(Response&&)>;

    ClientTransport(Connection *connection):
        connection_(connection)
    {}

    void onInput(StreamBuf& input_buffer) override;

    void writeRequest(const Request& request) override;

    void setResponseHandler(resp_handler_t&& handler);

private:
    Connection *connection_;
    resp_handler_t responseHandler_ = nullptr;
};

}
