#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include "Upgrader.h"

namespace shitty::http1 {

class UpgradeRegistry {
public:
    bool add(std::string_view token, std::unique_ptr<Upgrader>&& upgrader);

    const Upgrader* get(const std::string& token) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Upgrader>> upgrades_;
};

} // namespace
