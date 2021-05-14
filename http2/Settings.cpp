#include <arpa/inet.h>

#include "../Payload.h"
#include "Frame.h"
#include "Settings.h"

namespace shitty::http2 {

Settings Settings::createFromBuffer(std::span<char> buf) {
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

void Settings::writeTo(Payload& payload) const {
    constexpr uint16_t count = Settings::_Last - Settings::_First + 1;
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t nboSetting = htons(Settings::Name::_First + i);
        uint32_t nboValue = htonl(values_[i]);
        payload.write(&nboSetting, 2);
        payload.write(&nboValue, 4);
    }
}

} // namespace
