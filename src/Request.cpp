#include "Request.h"

using std::string;
using shitty::Request;

Request::Request(const string& method, const string& path, Message&& message):
    message_(std::move(message)),
    method_(method),
    path_(path)
{}
