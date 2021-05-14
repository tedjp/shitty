#pragma once

#include <array>
#include <string>

#include "StreamBuf.h"

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

// XXX: Replace this with a BufferedWriter that has an underlying Writer
// (anything with a write(buf, len) method). This is confusing.

// Proper use of a Payload *requires* calling send() on the destination
// connection; the StreamBuf target is only for buffering overflow - it doesn't
// handle notification that the socket needs to be written to.

class Payload {
public:
    Payload(StreamBuf* target);

    void write(const std::string& str);
    void send(const std::string& str);
    void write(const void *buf, size_t len);
    void writeOctet(uint8_t octet);
    void send(const void *buf, size_t len);
    const void *data() const;
    void *data();
    size_t size() const;
    size_t tailroom() const;
    bool empty() const;
    void clear();
    // WARNING: This is not the API you're after. Call send() on the underlying
    // connection. flush flushes to the outgoing StreamBuf but doesn't start
    // polling for output if the output buffer was already empty.
    void flush();

private:
    // Capacity 1440 is a common TCP/IPv6 MSS.
    constexpr static size_t capacity_ = 1440;

    size_t len_ = 0;
    std::array<char, capacity_> data_;
    StreamBuf* target_ = nullptr;
};

}

#include "Payload-inl.h"
