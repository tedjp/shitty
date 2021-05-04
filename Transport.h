#pragma once

#include "StreamBuf.h"

namespace shitty {

class StreamBuf;

class Transport {
public:
    Transport() = default;
    Transport(const Transport&) = default;
    Transport(Transport&&) = default;
    Transport& operator=(const Transport&) = default;
    Transport& operator=(Transport&&) = default;
    virtual ~Transport() = default;

    virtual void onInput(StreamBuf& buf) = 0;
};

}
