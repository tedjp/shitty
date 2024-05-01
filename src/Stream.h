#pragma once

#include "Headers.h"

namespace shitty {

class Stream {
public:
    Stream() = default;
    Stream(const Stream&) = default;
    Stream(Stream&&) = default;
    Stream& operator=(const Stream&) = default;
    Stream& operator=(Stream&&) = default;
    virtual ~Stream() = default;

    static void setGeneralServerHeaders(Headers& headers);

    static void setUserAgentHeader(Headers& headers);
};

}
