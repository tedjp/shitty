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
};

class ClientTransport: virtual public Transport {
public:
    virtual void writeRequest(const Request&) = 0;
};

class ServerTransport: virtual public Transport {
public:
    virtual void writeResponse(const Response&) = 0;
};

}
