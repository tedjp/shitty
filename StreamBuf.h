#pragma once

#include <vector>

namespace shitty {

class StreamBuf {
public:
    char* data() {
        return buf_.data();
    }

    const char* data() const {
        return buf_.data();
    }

    size_t size() const {
        return buf_.size();
    }

    size_t capacity() const {
        return buf_.capacity();
    }

    void clear() {
        buf_.clear();
    }

    // advance is not particularly efficient. It involves a memmove() unless the
    // entire buffer is being advanced.
    // This tends to be OK for non-pipelined HTTP/1 requests & responses.
    void advance(size_t amount);

    inline void reserve(size_t new_size) {
        buf_.reserve(new_size);
    }

    // Allocate more room (unspecified amount)
    void grow();

private:
    // Future: replace with 4 pointers like std::basic_streambuf/IOBuf:
    // start of allocation, read location, end of data, end of buffer.
    std::vector<char> buf_;
};

}
