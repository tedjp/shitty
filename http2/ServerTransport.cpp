#include <cassert>
#include <optional>
#include <unordered_map>

#include <fb64.h> // https://github.com/tedjp/fb64

#include "../Connection.h"
#include "Settings.h"
#include "ServerStream.h"
#include "ServerTransport.h"

using namespace std;

namespace shitty::http2 {

class ServerTransport::Impl {
public:
    Impl(
            Connection* connection,
            const Header& http2Settings,
            const Routes* routes);

    void onInput(StreamBuf& buf);

    ServerStream* getStream(uint32_t id);

    void sendPreface();
    void receivePreface(StreamBuf& buf);

    void receiveFrameHeader(StreamBuf& buf);
    void receiveFrameData(StreamBuf& buf);

    void ackSettings();

private:
    Connection* connection_;
    Settings localSettings_;
    Settings peerSettings_;
    const Routes* routes_;
    // unique_ptr indirection here is so that ServerStream
    // pointers/references are not invalidated by other streams being created or
    // destroyed in the same container (potential rehashing causing values to
    // move).
    unordered_map<uint32_t, unique_ptr<ServerStream>> streams_;

    // has_value when an entire frame header has been read, but the body has not
    // been completely processed.
    std::optional<FrameHeader> currentFrameHeader_;

    bool prefaceSent_ = false;
    bool prefaceReceived_ = false;
};

ServerTransport::ServerTransport(
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    impl_(make_unique<Impl>(connection, http2Settings, routes))
{}

void ServerTransport::onInput(StreamBuf& buf) {
    impl_->onInput(buf);
}

ServerStream* ServerTransport::getStream(uint32_t id) {
    return impl_->getStream(id);
}

ServerTransport::~ServerTransport()
{}

ServerTransport::Impl::Impl(
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    connection_(connection),
    routes_(routes)
{
    // decode settings
    {
        string_view settings = http2Settings.second;
        char decoded[128];
        const size_t decodedLen = fb64_decoded_size_nopad(settings.size());
        if (decodedLen > sizeof(decoded))
            throw runtime_error("excessive HTTP2-Settings length");

        int decodeResult = fb64_decode(
                settings.data(),
                settings.size(),
                reinterpret_cast<uint8_t*>(decoded));
        if (decodeResult != 0)
            throw runtime_error("HTTP2-Settings decode error");

        peerSettings_ = Settings::createFromBuffer(span(decoded, decodedLen));
    }

    streams_.emplace(1, make_unique<ServerStream>());
}

void ServerTransport::Impl::onInput(StreamBuf& buf) {
    // FIXME: This should be sent *once* *after* the HTTP/1.1 101 Switching
    // Protocols response. Sending it in the constructor is too soon (101 hasn't
    // been sent), and sending it on every input is _obviously_ wrong.
    // This entire class might need a separate path when constructing from
    // HTTP/1 upgrade (static constructor that handles the upgrade).
    if (!prefaceSent_)
        sendPreface();

    if (!prefaceReceived_)
        receivePreface(buf);

    if (!currentFrameHeader_.has_value())
        receiveFrameHeader(buf);
    else
        receiveFrameData(buf);
}

ServerStream* ServerTransport::Impl::getStream(uint32_t id) {
    auto it = streams_.find(id);
    if (it != streams_.end())
        return it->second.get();

    return nullptr;
}

void ServerTransport::Impl::sendPreface() {
    prefaceSent_ = true;

    // TODO: Replace with framing and stuff.
    const char settingsFrame[] = {
        0x00, 0x00, 0x00, // length (0)
        0x04, // type (SETTINGS)
        0x00, // flags (0)
        0x00, 0x00, 0x00, 0x00, // reserved bit, 31-bit stream id (must be 0 for SETTINGS)
    };

    connection_->send(settingsFrame, sizeof(settingsFrame) / sizeof(settingsFrame[0]));
}

void ServerTransport::Impl::receivePreface(StreamBuf& buf) {
    static constexpr uint8_t clientPreface[24]
        = {
            0x50, 0x52, 0x49, 0x20,  0x2a, 0x20, 0x48, 0x54,
            0x54, 0x50, 0x2f, 0x32,  0x2e, 0x30, 0x0d, 0x0a,
            0x0d, 0x0a, 0x53, 0x4d,  0x0d, 0x0a, 0x0d, 0x0a };

    if (buf.size() < sizeof(clientPreface))
        return; // come back later

    if (memcmp(buf.data(), clientPreface, sizeof(clientPreface)) != 0)
        throw std::runtime_error("Invalid connection preface"); // connection error

    prefaceReceived_ = true;

    buf.advance(sizeof(clientPreface));

    if (!buf.empty()) {
        // go straight into reading frames (probably settings to begin with, as
        // is required).
        receiveFrameHeader(buf);
    }
}

void ServerTransport::Impl::receiveFrameHeader(StreamBuf& buf) {
    assert(!currentFrameHeader_.has_value());

    if (buf.size() < FrameHeader::SIZE)
        return; // call back later

    currentFrameHeader_ = readFrameHeader(buf);
    const FrameHeader& header = currentFrameHeader_.value();

    if (buf.size() < header.length)
        return; // wait for more data

    receiveFrameData(buf);
}

void ServerTransport::Impl::receiveFrameData(StreamBuf& buf) {
    assert(currentFrameHeader_.has_value());
    const FrameHeader& header = currentFrameHeader_.value();

    if (buf.size() < header.length)
        return; // wait for data

    switch (header.type) {
    case FrameType::SETTINGS:
        if (!IsSettingsACK(header)) {
            peerSettings_ = Settings::createFromBuffer(span(buf.data(), header.length));
            ackSettings();
        }
        break;

    default:
        // ignore
        //std::cerr << "Unhandled frame type " << header.type << '\n';
        break;
    }

    buf.advance(header.length);
    // Prepare to read next frame header
    currentFrameHeader_.reset();
}

void ServerTransport::Impl::ackSettings() {
    FrameHeader header(FrameType::SETTINGS);
    header.flags = 0x01; // ACK

    Payload payload = connection_->getOutgoingPayload();

    writeFrameHeader(header, payload);

    connection_->send(payload);
}

} // namespace
