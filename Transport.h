#pragma once

namespace shitty {

class Request;
class Response;
class StreamBuf;

class Transport {
public:
    Transport() = default;
    Transport(const Transport&) = default;
    Transport(Transport&&) = default;
    Transport& operator=(const Transport&) = default;
    Transport& operator=(Transport&&) = default;
    virtual ~Transport();

    virtual void onInput(StreamBuf& buf) = 0;

protected:
    virtual void setGeneralHeaders(Headers&) {}
};

class ClientTransport: virtual public Transport {
public:
    virtual void sendRequest(const Request&) = 0;

protected:
    void setGeneralHeaders(Headers&) override;
};

class ServerTransport: virtual public Transport {
public:
    virtual void sendResponse(const Response&) = 0;

protected:
    void setGeneralHeaders(Headers&) override;
};

}
