#pragma once

#include <vector>

#include "Frame.h"

namespace shitty::http2 {

class HeadersFrame {
public:
    static bool isEndStream(const FrameHeader& frameHeader) { return frameHeader.flags.test(0); }
    static bool isEndHeaders(const FrameHeader& frameHeader) { return frameHeader.flags.test(2); }
    static bool isPadded(const FrameHeader& frameHeader) { return frameHeader.flags.test(3); }
    static bool isPriority(const FrameHeader& frameHeader) { return frameHeader.flags.test(5); }

    bool isEndStream() const { return isEndStream(frameHeader_); }
    bool isEndHeaders() const { return isEndHeaders(frameHeader_); }
    bool isPadded() const { return isPadded(frameHeader_); }
    bool isPriority() const { return isPriority(frameHeader_); }

    void setEndStream(bool value = true) { frameHeader_.flags.set(0, value); }
    void setEndHeaders(bool value = true) { frameHeader_.flags.set(2, value); }
    void setIsPadded(bool value = true) { frameHeader_.flags.set(3, value); }
    void setIsPriority(bool value = true) { frameHeader_.flags.set(5, value); }

    // Weight in the interface is in the range [1, 256].
    uint_fast16_t weight() const { return static_cast<uint_fast16_t>(weight_) + 1; }
    void setWeight(uint_fast16_t newWeight);

    void setHeaderBlockFragment(std::vector<uint8_t> headerBlockFragment);
    const std::vector<uint8_t>& headerBlockFragment() const { return headerBlockFragment_; }
    void setPadding(std::vector<uint8_t> padding);

private:
    // Many of the fields in the HeadersFrame depend on flags in the
    // FrameHeader.
    FrameHeader frameHeader_;

    uint8_t padLength_ = 0;

    // Stream dependency is only meaningful if the priority flag is set
    // (IsPriority()). Likewise for the weight field.
    unsigned exclusiveStreamDependency_ : 1 = 0;
    uint32_t streamDependency_ : 31 = 0;
    // weight is represented as the actual weight minus one; ie. the network
    // representation (one less than the "real" weight of 1-256).
    uint8_t weight_ = 15; // default from RFC 7540 5.3.5.

    std::vector<uint8_t> headerBlockFragment_;
    std::vector<uint8_t> padding_;
};

inline void HeadersFrame::setWeight(uint_fast16_t newWeight) {
    if (newWeight < 1 || newWeight > 256)
        throw std::runtime_error("invalid stream/header weight");

    weight_ = static_cast<uint8_t>(newWeight - 1);
}

void HeadersFrame::setHeaderBlockFragment(std::vector<uint8_t> headerBlockFragment) {
    headerBlockFragment_ = std::move(headerBlockFragment);
}

inline void HeadersFrame::setPadding(std::vector<uint8_t> padding) {
    padding_ = std::move(padding);
    setIsPadded(true);
}

} // namespace
