#include "ble_mac_util.h"
#include <cstdint>
#include <fstream>
#include <sstream>
#include <glog/logging.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <array>

/**
 * 시스템에서 hci 인터페이스의 MAC을 안전히 읽는 예시.
 * 실 환경에서는 ioctl(SIOCGIFADDR...) 또는 ip link show ... 등 호출 권고!
 */
std::array<uint8_t,6> BleMacUtil::getBleMacAddress(const std::string& iface) {
    // 1. sysfs 시도
    std::string filename = "/sys/class/bluetooth/" + iface + "/address";
    std::ifstream f(filename);
    std::string line;
    if (f && std::getline(f, line)) {
        std::array<uint8_t,6> mac{};
        std::istringstream iss(line);
        std::string byteStr;
        int i = 0;
        while (std::getline(iss, byteStr, ':') && i < 6) {
            mac[i++] = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
        }
        if (i == 6) return mac;
    }
    // 2. fallback: hciconfig 파싱
    std::array<uint8_t,6> mac{};
    std::string cmd = "hciconfig " + iface + " | grep Address | head -n 1";
    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) return mac;
    char buf[256];
    if (fgets(buf, sizeof(buf), fp)) {
        std::string line(buf);
        auto addr_pos = line.find("Address:");
        if (addr_pos != std::string::npos) {
            std::istringstream iss(line.substr(addr_pos + 8));
            std::string byteStr;
            int i = 0;
            while (std::getline(iss, byteStr, ':') && i < 6) {
                mac[i++] = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
            }
        }
    }
    pclose(fp);
    return mac;
}
