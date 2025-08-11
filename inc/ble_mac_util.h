#pragma once
#include <array>
#include <string>

class BleMacUtil {
public:
    // hci 인터페이스로부터 MAC주소를 읽음(실제 환경에서는 ioctl, system call 활용)
    static std::array<uint8_t,6> getBleMacAddress(const std::string& iface = "hci0");
};