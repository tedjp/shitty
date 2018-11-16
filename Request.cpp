#include "Request.h"

using std::string;
using shitty::Request;

Request::Request(const string& method, const string& path, Message&& message):
    message_(std::move(message)),
    method_(method),
    path_(path)
{}

#if 0
Request::Request(string&& method, string&& path, Headers&& headers):
    method_(std::move(method)),
    path_(std::move(path)),
    headers_(std::move(headers))
{
}

Request::Request(const string& method, const string& path, Headers&& headers):
    method_(method),
    path_(path),
    message_(std::move(headers))
{
}
#endif
