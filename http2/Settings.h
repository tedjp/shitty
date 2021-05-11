#pragma once

#include <span>
#include <stdexcept>

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

    uint32_t values_[Name::_Last - Name::_First + 1] = {};

    uint32_t& value(Name name) { return values_[name - Name::_First]; }
    uint32_t value(Name name) const { return values_[name - Name::_First]; }

    static Settings createFromBuffer(std::span<char> buf);
};

inline Settings Settings::createFromBuffer(std::span<char> buf) {
    if (buf.size() % 6 != 0)
        throw std::runtime_error("malformed SETTINGS: size not a multiple of 6");

    Settings settings;

    for (; !buf.empty(); buf = buf.subspan(6)) {
        std::span<char> setting = buf.subspan(0, 6);
        uint16_t id = static_cast<uint16_t>(setting[0]) << 8 | static_cast<uint16_t>(setting[1]);
        if (id < Settings::Name::_First || id > Settings::Name::_Last)
            continue;
        uint32_t value
            = static_cast<uint32_t>(setting[2]) << 24
            | static_cast<uint32_t>(setting[3]) << 16
            | static_cast<uint32_t>(setting[4]) <<  8
            | static_cast<uint32_t>(setting[5]) <<  0;
        settings.value(Settings::Name(id)) = value;
    }

    return settings;
}

} // namespace
