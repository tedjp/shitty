#include <cstdarg>

#include "Response.h"

using std::string;
using shitty::Response;
using shitty::Header;

Response::Response(string&& body, std::initializer_list<string> headers):
    message(std::move(body), std::move(headers))
{}

Response::Response(string&& body, std::initializer_list<Header> headers):
    message(std::move(body), std::move(headers))
{}

Response::Response(std::initializer_list<string> headers, std::string&& body):
    message(std::move(headers), std::move(body))
{}

Response::Response(std::initializer_list<Header> headers, std::string&& body):
    message(std::move(headers), std::move(body))
{}
