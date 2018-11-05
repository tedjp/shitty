#include "Message.h"

using shitty::Message;

Message::Message(std::string&& body):
    body_(std::move(body))
{}

Message::Message(Headers&& headers):
    headers_(std::move(headers))
{}

Message::~Message()
{}
