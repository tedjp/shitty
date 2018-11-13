#include <array>
#include <cstring>
#include <ctime>

#include "Error.h"
#include "HTTPDate.h"

using shitty::HTTPDate;

static constexpr std::array<char[4], 7> daysOfWeek = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

static constexpr std::array<char[4], 12> months = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

std::string HTTPDate::now() {
    std::string str{"Sun, 06 Nov 1994 08:49:37 GMT"};

    time_t tt = time(nullptr);
    struct tm tm;

    if (gmtime_r(&tt, &tm) == nullptr)
        throw error_errno("gmtime_r");

    // strftime() is subject to the current locale, so we build the date string
    // ourselves.
    // Assigning digits piecemeal is significantly faster than using strftime()
    // to output the numeric values.
    const char *dayOfWeek = daysOfWeek[tm.tm_wday];
    const char *month = months[tm.tm_mon];

    memcpy(&str[0], dayOfWeek, 3);

    str[5] = '0' + tm.tm_mday / 10;
    str[6] = '0' + tm.tm_mday % 10;

    memcpy(&str[8], month, 3);

    tm.tm_year += 1900;

    // Year 10,000: Check how the HTTP spec wants to handle 5-digit years.
    str[12] = '0' + tm.tm_year        / 1000;
    str[13] = '0' + tm.tm_year % 1000 / 100;
    str[14] = '0' + tm.tm_year %  100 / 10;
    str[15] = '0' + tm.tm_year %   10;

    str[17] = '0' + tm.tm_hour / 10;
    str[18] = '0' + tm.tm_hour % 10;

    str[20] = '0' + tm.tm_min / 10;
    str[21] = '0' + tm.tm_min % 10;

    str[23] = '0' + tm.tm_sec / 10;
    str[24] = '0' + tm.tm_sec % 10;

    return str;
}
