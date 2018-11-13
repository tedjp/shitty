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

    char numeric_fields[sizeof("YYYYDDHHMMSS")];
    const char fmt_str[] = "%Y%d%H%M%S";
    ssize_t len = strftime(numeric_fields, sizeof(numeric_fields), fmt_str, &tm);
    if (len != sizeof(numeric_fields) - 1)
        throw std::logic_error("Failed to format numeric date fields");

    const char *dayOfWeek = daysOfWeek[tm.tm_wday];
    const char *month = months[tm.tm_mon];

    memcpy(&str[0], dayOfWeek, 3);
    memcpy(&str[5], &numeric_fields[4], 2); // day-of-month (number)
    memcpy(&str[8], month, 3);
    memcpy(&str[12], &numeric_fields[0], 4); // year
    memcpy(&str[17], &numeric_fields[6], 2); // hour
    memcpy(&str[20], &numeric_fields[8], 2); // minute
    memcpy(&str[23], &numeric_fields[10], 2); // seconnds

    return str;
}
