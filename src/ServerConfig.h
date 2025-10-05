#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace shitty {

class ServerConfig {
public:
    static constexpr std::array<uint16_t, 2> default_ports_{ 80, 8080 };

    ServerConfig();
    ServerConfig(ServerConfig&&) = default;
    ServerConfig(const ServerConfig&) = default;
    ServerConfig& operator=(const ServerConfig&) = default;
    ServerConfig& operator=(ServerConfig&&) = default;

    ServerConfig& setBindPort(const uint16_t port);
    ServerConfig& setBindPorts(std::span<const uint16_t> ports);

    std::span<const uint16_t> viewBindPorts() const;

    virtual ~ServerConfig() = default;

private:
    std::vector<uint16_t> bind_ports_;
};

inline ServerConfig::ServerConfig():
    bind_ports_(default_ports_.begin(), default_ports_.end())
{}

inline ServerConfig& ServerConfig::setBindPort(const uint16_t port) {
    bind_ports_.clear();
    bind_ports_.emplace_back(port);
    return *this;
}

inline ServerConfig& ServerConfig::setBindPorts(std::span<const uint16_t> ports) {
    bind_ports_ = std::vector<uint16_t>(ports.begin(), ports.end());
    return *this;
}

inline std::span<const uint16_t> ServerConfig::viewBindPorts() const {
    return bind_ports_;
}

} // namespace shitty