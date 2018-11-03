#include "headers.h"

using std::string;
using shitty::Headers;

void Headers::set(const string& name, const string& value) {
    kv_.erase(name);
    kv_.emplace(name, value);
}

void Headers::add(const string& name, const string& value) {
    kv_.emplace(name, value);
}
