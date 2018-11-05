#include "Request.h"

using std::string;
using shitty::Request;

Request::Request(const string& method, const string& path, Headers&& headers):
    method_(method),
    path_(path),
    message_(std::move(headers))
{
}
