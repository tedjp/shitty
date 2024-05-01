#include "Message.h"

using shitty::Message;

Message::Message(std::string&& body):
    body_(std::move(body))
{}

Message::Message(Headers&& headers):
    headers_(std::move(headers))
{}

Message::Message(std::string&& body, std::initializer_list<std::string> headers):
    body_(std::move(body)),
    headers_(std::move(headers))
{}

Message::Message(std::string&& body, std::initializer_list<Header> headers):
    body_(std::move(body)),
    headers_(std::move(headers))
{}

Message::Message(std::initializer_list<std::string> headers, std::string&& body):
    body_(std::move(body)),
    headers_(std::move(headers))
{}

Message::Message(std::initializer_list<Header> headers, std::string&& body):
    body_(std::move(body)),
    headers_(std::move(headers))
{}

Message::Message(const std::string& body, const Headers& headers):
    body_(body),
    headers_(headers)
{}

Message::Message(Headers&& headers, std::span<const std::byte> body):
    body_(std::string(reinterpret_cast<const char*>(body.data()), body.size())),
    headers_(std::move(headers))
{}
