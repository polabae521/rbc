#pragma once
#include <string>
#include <array>
#include <cstdint>

class BleMacUtil {
public:
    static std::array<uint8_t,6> getBleMacAddress(const std::string& iface = "hci0");
};