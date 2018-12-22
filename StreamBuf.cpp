#include <cstring>
#include <limits>
#include <stdexcept>

#include "StreamBuf.h"

using shitty::StreamBuf;

void StreamBuf::advance(size_t amount) {
    if (amount > size())
        throw std::length_error("StreamBuf::advance out of range");

    if (amount == size()) {
        clear();
        return;
    }

    // probably faster than std::copy().
    in_use_ -= amount;
    memmove(buf_.data(), buf_.data() + amount, in_use_);
}

void StreamBuf::grow() {
    size_t newcap;

    static const size_t GROWTH_FACTOR = 4;

    if (size() < 4096 / GROWTH_FACTOR) {
        newcap = 4096;
    } else if (size() > capacity() / 2) {
        if (capacity() > std::numeric_limits<size_t>::max() / GROWTH_FACTOR)
            newcap = std::numeric_limits<size_t>::max();
        else
            newcap = capacity() * GROWTH_FACTOR;
    } else {
        return; // plenty of space still
    }

    reserve(newcap);
}

void StreamBuf::write(const void *data, size_t len) {
    ensure(len);
    memcpy(tail(), data, len);
    addTailContent(len);
}
