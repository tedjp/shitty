#include <cstring>
#include <stdexcept>

#include "StreamBuf.h"

using shitty::StreamBuf;

void StreamBuf::advance(size_t amount) {
    if (amount > size())
        throw std::length_error("StreamBuf::advance out of range");

    if (amount == size()) {
        buf_.clear();
        return;
    }

    // probably faster than std::copy().
    memmove(buf_.data(), buf_.data() + amount, buf_.size() - amount);
    buf_.resize(buf_.size() - amount);
}

void StreamBuf::grow() {
    size_t newcap;

    if (size() < 4096 / 2)
        newcap = 4096;
    else
        newcap = size() * 2;

    reserve(newcap);
}
