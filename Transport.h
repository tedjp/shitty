#pragma once

#include "Response.h"
#include "StreamBuf.h"

namespace shitty {

class Transport {
public:
    virtual void onInput(StreamBuf& buf) = 0;

    virtual void writeResponse(const Response& r) = 0;

    virtual ~Transport();
};

}
