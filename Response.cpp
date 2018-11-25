#include <cstdarg>

#include "Response.h"

using std::string;
using shitty::Response;
using shitty::Header;

Response::Response(unsigned status_code, Message&& msg):
    message(std::move(msg)),
    status_code_(status_code)
{}

Response::Response(std::initializer_list<string> headers, std::string&& body):
    message(std::move(headers), std::move(body))
{}

Response::Response(std::initializer_list<Header> headers, std::string&& body):
    message(std::move(headers), std::move(body))
{}
