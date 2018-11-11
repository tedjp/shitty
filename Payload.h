#pragma once

namespace shitty {

// Corking
// Rather than corking & uncorking a connection,
// which is hard to write in an exception-safe manner,
// construct a Payload (preferably on the stack) and write to it; then call
// Payload::flush() when you're done.
// (The Payload is *not* flushed by its destructor.)
// A Payload does not allocate any heap memory of its own, but must be
// provided a StreamBuf to write to if it overflows. The StreamBuf, in turn,
// might allocate heap memory to buffer the data.
// Due to the possibility of insufficient network buffer for large writes,
// the options for an overflowing write are:
// (a) fail the write (this is a bad interface)
// (b) allocate its own heap space to overflow into; or
// (c) use an existing overflow buffer (such as an existing StreamBuf).
// The benefit of (c) is that if the outgoing buffer has sufficient
// capacity, an overflowing Payload does not necessarily cause a heap
// allocation.
// Payloads are ideal for lots of small writes (eg. HTTP headers) rather
// than large amounts of data (message bodies). Overflowing the Payload can
// cause unnecessary thrashing as each block is written to the Payload,
// evicted to the growable StreamBuf, and replaced with the next data,
// repeatedly.
// It might be useful to allow Payloads to be stacked; for one to overflow into
// another, eg. for constructing an HTTP header line into a Payload which then
// is constructed into a Payload containing all headers, before being sent. That
// probably involves having Payload & StreamBuf inherit from a common abstract
// base Writer.
// Default Capacity 1440 is a common TCP/IPv6 MSS.
// XXX: Is this better than just writing straight to the StreamBuf?

template <size_t Capacity = 1440>
class Payload {
public:
    Payload(StreamBuf* target);
    void write(const std::string& str);
    void send(const std::string& str);
    void write(const void *buf, size_t len);
    void send(const void *buf, size_t len);
    const void *data() const;
    void *data();
    size_t size() const;
    size_t tailroom() const;
    bool empty() const;
    void clear();
    void flush();

private:
    size_t len_ = 0;
    std::array<char, Capacity> data_;
    StreamBuf* target_ = nullptr;
};

}

#include "Payload-inl.h"
