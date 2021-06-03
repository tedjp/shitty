#include <cassert>
#include <optional>
#include <unordered_map>

#include <fb64/fb64.h> // https://github.com/tedjp/fb64
#include <hpack/header.h>

#include "../Connection.h"
#include "DataFrame.h"
#include "FlowControl.h"
#include "HeadersFrame.h"
#include "Protocol.h"
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

    Impl(ServerTransport* parent, Connection* connection);

    void onInput(StreamBuf& buf);

    // Get a ServerStream. Throw if it doesn't exist.
    ServerStream& getStream(uint32_t id);

    void writeFrame(FrameHeader frameHeader, std::span<const std::byte> data);
    void writeHeadersFrame(const HeadersFrame& frame);

private:
    void initialize();

    void sendPreface();
    void receivePreface(StreamBuf& buf);

    void receiveFrames(StreamBuf& buf);
    bool receiveFrame(StreamBuf& buf);

    void receiveFrameHeader(StreamBuf& buf);

    void ackSettings();

    // Frame type handlers
    void receiveData(const FrameHeader& frameHeader, StreamBuf& buf);
    void receiveHeaders(const FrameHeader& frameHeader, StreamBuf& buf);
    void receivePing(const FrameHeader& header, StreamBuf& buf);
    void receiveResetStream(const FrameHeader& header, StreamBuf& buf);
    void receiveSettings(const FrameHeader& header, StreamBuf& buf);
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

ServerTransport::ServerTransport(Connection* connection):
    impl_(make_unique<Impl>(this, connection))
{}

void ServerTransport::onInput(StreamBuf& buf) {
    impl_->onInput(buf);
}

void ServerTransport::writeFrame(FrameHeader frameHeader, std::span<const std::byte> data) {
    impl_->writeFrame(std::move(frameHeader), data);
}

void ServerTransport::writeHeadersFrame(const HeadersFrame& frame) {
    impl_->writeHeadersFrame(frame);
}

ServerStream& ServerTransport::getStream(uint32_t id) {
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
    initialize();
}

ServerTransport::Impl::Impl(ServerTransport* parent, Connection* connection):
    parent_(parent),
    connection_(connection)
{
    initialize();
}

void ServerTransport::Impl::initialize() {
    // Disable PUSH_PROMISE due to no handling.
    localSettings_[Settings::EnablePush] = 0;

    streams_.emplace(1u, make_unique<ServerStream>(1u, parent_));

    sendPreface();
}

void ServerTransport::Impl::onInput(StreamBuf& buf) {
    if (!prefaceReceived_)
        receivePreface(buf);

    receiveFrames(buf);
}

ServerStream& ServerTransport::Impl::getStream(uint32_t id) {
    auto it = streams_.find(id);
    if (it != streams_.end())
        return *it->second.get();

    throw runtime_error("No such stream");
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
    using protocol::CLIENT_PREFACE;

    if (buf.size() < CLIENT_PREFACE.size())
        return; // come back later

    if (memcmp(buf.data(), CLIENT_PREFACE.data(), CLIENT_PREFACE.size()) != 0)
        throw std::runtime_error("Invalid connection preface"); // connection error

    prefaceReceived_ = true;

    buf.advance(CLIENT_PREFACE.size());

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
    case FrameType::DATA:
        receiveData(header, buf);
        break;

    case FrameType::HEADERS:
        receiveHeaders(header, buf);
        break;

    case FrameType::PING:
        receivePing(header, buf);
        break;

    case FrameType::RST_STREAM:
        receiveResetStream(header, buf);
        break;

    case FrameType::SETTINGS:
        receiveSettings(header, buf);
        break;

    case FrameType::WINDOW_UPDATE:
        receiveWindowUpdate(header, buf);
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

static void sendPingResponse(Connection& connection, span<const char> data) {
    FrameHeader header(FrameType::PING);
    header.flags.set(0); // ACK
    header.length = data.size();

    Payload payload = connection.getOutgoingPayload();

    header.writeTo(payload);

    // Copy the incoming ping buffer back to the sender
    payload.write(data.data(), data.size());

    connection.send(payload);
}

void ServerTransport::Impl::receivePing(
        const FrameHeader& header,
        StreamBuf& buf)
{
    if (isPingACK(header))
        return;

    constexpr unsigned PAYLOAD_SIZE = 8;

    if (header.streamId != 0) {
        // Connection error: PROTOCOL_ERROR (RFC 7540 6.7)
        throw runtime_error(
                "Non-zero stream ID " + to_string(header.streamId) + " in PING");
    }

    if (header.length != PAYLOAD_SIZE) {
        // Connection error: FRAME_SIZE_ERROR (RFC 7540 6.7)
        throw runtime_error(
                "Incorrect PING payload size " + to_string(header.length));
    }

    if (buf.size() < PAYLOAD_SIZE)
        return; // come again

    sendPingResponse(*connection_, span(buf.data(), PAYLOAD_SIZE));

    buf.advance(PAYLOAD_SIZE);
}

void ServerTransport::Impl::receiveResetStream(
        const FrameHeader& header,
        StreamBuf& buf) {
    if (header.length != 4)
        throw runtime_error("RST_STREAM wrong size");

    if (header.streamId == 0) // connection error: PROTOCOL_ERROR
        throw runtime_error("Invalid RST_STREAM stream id 0");

    const uint32_t errorCode
        = static_cast<uint32_t>(buf.data()[0] << 24)
        | static_cast<uint32_t>(buf.data()[1] << 16)
        | static_cast<uint32_t>(buf.data()[2] <<  8)
        | static_cast<uint32_t>(buf.data()[3] <<  0);

    if (errorCode != 0) {
        // TODO: Return error to caller (if this is an outgoing stream).
    }

    streams_.erase(header.streamId);
}

void ServerTransport::Impl::receiveSettings(
        const FrameHeader& header,
        StreamBuf& buf)
{
    if (isSettingsACK(header))
        return;

    peerSettings_ = Settings::createFromBuffer(span(buf.data(), header.length));

    if (!windowUpdateReceived_)
        windowSize_ = peerSettings_.value(Settings::InitialWindowSize);

    ackSettings();
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

void ServerTransport::Impl::receiveData(
        const FrameHeader& frameHeader,
        StreamBuf& buf) {
    if (frameHeader.streamId == 0)
        throw runtime_error("connection_error PROTOCOL_ERROR");

    ServerStream& stream = getStream(frameHeader.streamId);

    // Check state is Open or half-closed (local)
    if (stream.getState() != StreamState::Open
            && stream.getState() != StreamState::HalfClosedLocal)
        throw runtime_error("stream_error STREAM_CLOSED");

    // TODO: Handle exception as stream error (spec does not specify)
    stream.subtractWindowSize(frameHeader.length);

    const bool padded = DataFrame::isPadded(frameHeader);
    uint32_t length = frameHeader.length;

    if (padded) {
        if (length == 0 || buf.empty()) // connection error, protocol error (not specified)
            throw runtime_error("Empty padded data frame");

        char charPadLength = buf.data()[0];
        buf.advance(1);

        uint8_t padLength = static_cast<uint8_t>(charPadLength);

        if (length < padLength) // connection error, protocol error (not specified)
            throw runtime_error("Padding length exceeds DATA frame size");

        length -= padLength;
    }

    // TODO: Handle actual data

    if (DataFrame::isEndStream(frameHeader)) {
        // TODO: Accumulate request & headers and pass to handler
        stream.onRequest(Request());
        endStream(frameHeader.streamId);
    }
}

void ServerTransport::Impl::receiveHeaders(
        const FrameHeader& frameHeader,
        StreamBuf& buf) {
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
