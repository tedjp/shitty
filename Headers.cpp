#include "Headers.h"

using std::string;
using shitty::Headers;

void Headers::set(const string& name, const string& value) {
    // TODO: case-fold here.
    kv_.erase(name);
    kv_.emplace(name, value);
}

void Headers::add(const string& name, const string& value) {
    // TODO: case-fold here.
    kv_.emplace(name, value);
}
