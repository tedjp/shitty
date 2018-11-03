#include <cstdarg>

#include "static_response.h"

using std::string;
using shitty::Header;
using shitty::Headers;
using shitty::StaticResponse;

StaticResponse::StaticResponse(const string& body):
    body_(body),
    headers_()
{
}

StaticResponse::StaticResponse(const string& body, const Headers& headers):
    body_(body),
    headers_(headers)
{
}

#if 0
StaticResponse::StaticResponse(const string& body, ...) {
    body_(body),
    headers_()
{
    va_list ap;
    va_start(ap, body);
    headers_.add(

}
#endif
