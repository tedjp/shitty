#pragma once

#include "Headers.h"
#include "StreamBuf.h"

namespace shitty {

class Request;
class Response;
class StreamBuf;

class Transport {
public:
    Transport() = default;
    Transport(const Transport&) = default;
    Transport(Transport&&) = default;
    Transport& operator=(const Transport&) = default;
    Transport& operator=(Transport&&) = default;
    virtual ~Transport();

    virtual void onInput(StreamBuf& buf) = 0;

protected:
    virtual void setGeneralHeaders(Headers&) {}
};

}
