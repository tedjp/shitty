#pragma once

#include <cstdint>
#include <vector>

namespace shitty {

// A buffer that can be written into up to capacity().
// size() is how much has been written to.
// Unlike a std::vector, it's safe to write into the space between size() &
// capacity() so long as you update size() afterwards.
class StreamBuf {
public:
    char* data() {
        return buf_.data();
    }

    const char* data() const {
        return buf_.data();
    }

    size_t size() const {
        return in_use_;
    }

    bool empty() const {
        return size() == 0;
    }

    bool isEmpty() const {
        return size() == 0;
    }

    size_t capacity() const {
        return buf_.size();
    }

    size_t tailroom() const {
        return capacity() - size();
    }

    // where to append to
    char *tail() {
        return buf_.data() + in_use_;
    }

    // This is a bad name.
    void addTailContent(size_t size) {
        in_use_ += size;
    }

    void clear() {
        in_use_ = 0;
    }

    void write(const void *data, size_t len);

    void writeOctet(uint8_t octet);

    // Avoid doing this TBH.
    void shrink_to_fit() {
        buf_.resize(in_use_);
        buf_.shrink_to_fit();
    }

    // advance is not particularly efficient. It involves a memmove() unless the
    // entire buffer is being advanced.
    // This tends to be OK for non-pipelined HTTP/1 requests & responses.
    void advance(size_t amount);

    inline void reserve(size_t new_size) {
        if (new_size < buf_.size())
            return;

        buf_.resize(new_size);
    }

    void ensure(size_t extra_capacity) {
        reserve(size() + extra_capacity);
    }

    // Allocate more room (unspecified amount)
    void grow();

private:
    // Future: replace with 4 pointers like std::basic_streambuf/IOBuf:
    // start of allocation, read location, end of data, end of buffer.
    // vector is really inefficient because it zeroes-out the memory of the
    // buffer before we write into it, discarding what was there anyway.
    // This should at least be replaced with malloc/realloc.
    size_t in_use_ = 0;
    std::vector<char> buf_;
};

}
