#include <cassert>
#include <optional>
#include <unordered_map>

#include <fb64/fb64.h> // https://github.com/tedjp/fb64
#include <hpack/header.h>

#include "../Connection.h"
#include "FlowControl.h"
#include "HeadersFrame.h"
#include "Settings.h"
#include "ServerStream.h"
#include "ServerTransport.h"

using namespace std;

namespace shitty::http2 {

class ServerTransport::Impl {
public:
    Impl(
            ServerTransport* parent,
            Connection* connection,
            const Header& http2Settings,
            const Routes* routes);

    void onInput(StreamBuf& buf);

    ServerStream* getStream(uint32_t id);

    void sendPreface();
    void receivePreface(StreamBuf& buf);

    void writeFrame(FrameHeader frameHeader, std::span<const std::byte> data);
    void writeHeadersFrame(const HeadersFrame& frame);

private:
    void receiveFrames(StreamBuf& buf);
    bool receiveFrame(StreamBuf& buf);

    void receiveFrameHeader(StreamBuf& buf);

    void ackSettings();

    // Frame type handlers
    void receiveHeaders(StreamBuf& buf);
    void receiveWindowUpdate(const FrameHeader& frameHeader, StreamBuf& buf);

    void addConnectionWindowSize(int32_t increment);
    void addStreamWindowSize(uint32_t streamId, int32_t increment);

    void processFrameBody(StreamBuf& buf);
    void endStream(uint32_t streamId);

    ServerTransport* parent_ = nullptr;
    Connection* connection_ = nullptr;
    Settings localSettings_;
    Settings peerSettings_;
    const Routes* routes_ = nullptr;

    // XXX: HPACK symbols ought to be namespaced properly
    HeaderDecoder headerDecoder_;

    // unique_ptr indirection here is so that ServerStream
    // pointers/references are not invalidated by other streams being created or
    // destroyed in the same container (potential rehashing causing values to
    // move).
    unordered_map<uint32_t, unique_ptr<ServerStream>> streams_;

    // has_value when an entire frame header has been read, but the body has not
    // been completely processed.
    std::optional<FrameHeader> currentFrameHeader_;

    // connection window size
    uint32_t windowSize_ = 65535; // RFC 7540 6.9.2

    // TODO: Shrink to bits
    bool prefaceReceived_ = false;
    bool windowUpdateReceived_ = false;
};

ServerTransport::ServerTransport(
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    impl_(make_unique<Impl>(this, connection, http2Settings, routes))
{}

void ServerTransport::sendPreface() {
    impl_->sendPreface();
}

void ServerTransport::onInput(StreamBuf& buf) {
    impl_->onInput(buf);
}

void ServerTransport::writeFrame(FrameHeader frameHeader, std::span<const std::byte> data) {
    impl_->writeFrame(std::move(frameHeader), data);
}

void ServerTransport::writeHeadersFrame(const HeadersFrame& frame) {
    impl_->writeHeadersFrame(frame);
}

ServerStream* ServerTransport::getStream(uint32_t id) {
    return impl_->getStream(id);
}

ServerTransport::~ServerTransport()
{}

static Settings decodeBase64Settings(std::string_view b64encoded) {
    char decoded[128];
    const size_t decodedLen = fb64_decoded_size_nopad(b64encoded.size());
    if (decodedLen > sizeof(decoded))
        throw runtime_error("excessive HTTP2-Settings length");

    int decodeResult = fb64_decode(
            b64encoded.data(),
            b64encoded.size(),
            reinterpret_cast<uint8_t*>(decoded));
    if (decodeResult != 0)
        throw runtime_error("HTTP2-Settings decode error");

    return Settings::createFromBuffer(span(decoded, decodedLen));
}

ServerTransport::Impl::Impl(
        ServerTransport* parent,
        Connection* connection,
        const Header& http2Settings,
        const Routes* routes):
    parent_(parent),
    connection_(connection),
    peerSettings_(decodeBase64Settings(http2Settings.second)),
    routes_(routes)
{
    streams_.emplace(1u, make_unique<ServerStream>(1u, parent_));
}

void ServerTransport::Impl::onInput(StreamBuf& buf) {
    if (!prefaceReceived_)
        receivePreface(buf);

    receiveFrames(buf);
}

ServerStream* ServerTransport::Impl::getStream(uint32_t id) {
    auto it = streams_.find(id);
    if (it != streams_.end())
        return it->second.get();

    return nullptr;
}

void ServerTransport::Impl::sendPreface() {
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

    receiveFrames(buf);
}

void ServerTransport::Impl::receiveFrames(StreamBuf& buf) {
    bool moreToProcess;
    do {
        moreToProcess = receiveFrame(buf);
    } while (moreToProcess && !buf.isEmpty());
}

bool ServerTransport::Impl::receiveFrame(StreamBuf& buf) {
    if (!currentFrameHeader_.has_value())
        receiveFrameHeader(buf);
    if (!currentFrameHeader_.has_value())
        return false; // wait for data
    if (buf.size() < currentFrameHeader_.value().length)
        return false; // wait for frame body
    processFrameBody(buf);
    return true; // maybe another frame to process - call again
}

void ServerTransport::Impl::receiveFrameHeader(StreamBuf& buf) {
    assert(!currentFrameHeader_.has_value());

    if (buf.size() < FrameHeader::SIZE)
        return; // call back later

    currentFrameHeader_ = readFrameHeader(buf);
    const FrameHeader& header = currentFrameHeader_.value();

    if (buf.size() < header.length)
        return; // wait for more data
}

void ServerTransport::Impl::processFrameBody(StreamBuf& buf) {
    assert(currentFrameHeader_.has_value());
    const FrameHeader& header = currentFrameHeader_.value();

    if (buf.size() < header.length)
        return; // wait for data

    switch (header.type) {
    case FrameType::SETTINGS:
        if (!IsSettingsACK(header)) {
            peerSettings_ = Settings::createFromBuffer(span(buf.data(), header.length));

            if (!windowUpdateReceived_)
                windowSize_ = peerSettings_.value(Settings::InitialWindowSize);

            ackSettings();
        }
        break;

    case FrameType::WINDOW_UPDATE:
        receiveWindowUpdate(header, buf);
        break;

    case FrameType::HEADERS:
        receiveHeaders(buf);
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
    header.flags.set(0); // ACK

    Payload payload = connection_->getOutgoingPayload();

    header.writeTo(payload);

    connection_->send(payload);
}

// Write a frame. The length field will be set to the length of the data.
void ServerTransport::Impl::writeFrame(FrameHeader frameHeader, std::span<const std::byte> data) {
    frameHeader.length = data.size();

    Payload payload = connection_->getOutgoingPayload();
    frameHeader.writeTo(payload);
    payload.write(data.data(), data.size());

    connection_->send(payload);
}

void ServerTransport::Impl::writeHeadersFrame(const HeadersFrame& frame) {
    Payload payload = connection_->getOutgoingPayload();

    frame.frameHeader().writeTo(payload);
    payload.write(frame.headerBlockFragment());
    payload.write(frame.padding());
    connection_->send(payload);
}

void ServerTransport::Impl::receiveHeaders(StreamBuf& buf) {
    assert(currentFrameHeader_.has_value());
    const FrameHeader& frameHeader = currentFrameHeader_.value();
    assert(buf.size() >= frameHeader.length);

    // XXX: HPACK ::Header is not the same as shitty::Header
    vector<::Header> headers = headerDecoder_.parseHeaders(
            span(reinterpret_cast<const std::byte*>(buf.data()), frameHeader.length));

    ServerStream* stream = nullptr;

    auto streamIt = streams_.find(frameHeader.streamId);
    if (streamIt != streams_.end())
        stream = streamIt->second.get();
    else {
        auto [it, inserted] = streams_.emplace(
                frameHeader.streamId,
                make_unique<ServerStream>(frameHeader.streamId, parent_));
        assert(inserted);
        stream = it->second.get();
    }

    assert(stream != nullptr);

    // TODO: Accumulate headers until IsEndHeaders
    //stream->headers_.add(headers);

    // TODO: Accumulate body until IsEndStream
    // For now just ignore everything until the EndStream & EndHeaders frame
    // (and ignore its body too)
    if (HeadersFrame::isEndHeaders(frameHeader)
            && HeadersFrame::isEndStream(frameHeader))
    {
        // TODO: provide real request to OnRequest method
        stream->onRequest(Request());
        // Clean up stream
        endStream(frameHeader.streamId);
    }
}

static int32_t readWindowSize(StreamBuf& buf) {
    // 1 reserved bit, 31 "Window Size Increment" bits
    int32_t windowSize = 0;

    windowSize
        = buf.data()[0] << 24
        | buf.data()[1] << 16
        | buf.data()[2] <<  8
        | buf.data()[3] <<  0;

    // ignore reserved bit
    windowSize &= 0x7fffffff;

    return windowSize;
}

void ServerTransport::Impl::receiveWindowUpdate(
        const FrameHeader& header,
        StreamBuf& buf)
{
    if (header.length != 4) // TODO: connection error FRAME_SIZE_ERROR
        throw runtime_error("WINDOW_UPDATE wrong size");

    int32_t windowSize = readWindowSize(buf);

    if (header.streamId == 0)
        addConnectionWindowSize(windowSize);
    else
        addStreamWindowSize(header.streamId, windowSize);
}

void ServerTransport::Impl::addConnectionWindowSize(int32_t windowSize) {
    try {
        windowSize_ = addWindowSize(windowSize_, windowSize);
    } catch (std::runtime_error& err) {
        // FIXME: This is a connection error (6.9), handle
        // accordingly (5.4.1)
        throw std::runtime_error(
                string("connection error: ") + err.what());
    }

    windowUpdateReceived_ = true;
}

void ServerTransport::Impl::addStreamWindowSize(uint32_t streamId, int32_t windowSize) {
    auto stream = streams_.find(streamId);

    if (stream != streams_.end())
        stream->second->addWindowSize(windowSize);
}

void ServerTransport::Impl::endStream(uint32_t streamId) {
    streams_.erase(streamId);
}

} // namespace
