#include "UpgradeRegistry.h"

using namespace std;

namespace shitty::http1 {

bool UpgradeRegistry::add(string_view token, unique_ptr<Upgrader>&& upgrader)
{
    return
        upgrades_
        .try_emplace(string(token), move(upgrader))
        .second;
}

const Upgrader* UpgradeRegistry::get(const string& token) const {
    auto it = upgrades_.find(token);
    return it == upgrades_.end() ? nullptr : it->second.get();
}

} // shitty
