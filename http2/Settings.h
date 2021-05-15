#pragma once

#include <span>
#include <stdexcept>

namespace shitty {
class Payload;
}

namespace shitty::http2 {

struct Settings {
    enum Name : uint16_t {
        HeaderTableSize = 0x01,
        _First = 0x01,
        EnablePush = 0x02,
        MaxConcurrentStreams = 0x03,
        InitialWindowSize = 0x04,
        MaxFrameSize = 0x05,
        MaxHeaderListSize = 0x06,
        _Last = 0x06
    };

    static constexpr size_t serializedSize = (_Last - _First + 1) * 6;

    uint32_t values_[Name::_Last - Name::_First + 1] =
    {
        4096,
        1,
        ~0u, // "Initially, there is no limit to this value."
        65535,
        16384,
        ~0u, // "The initial value of this setting is unlimited."
    };

    uint32_t& value(Name name) { return values_[name - Name::_First]; }
    uint32_t value(Name name) const { return values_[name - Name::_First]; }

    static Settings createFromBuffer(std::span<char> buf);

    // Does not include the frame header
    void writeTo(Payload& payload) const;
};

} // namespace
