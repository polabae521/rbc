#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>

class Advertiser {
    std::string hci_iface_;
    std::thread adv_thread_;
    std::atomic<bool> running_{false};
    std::function<std::vector<uint8_t>()> packet_generator_;
public:
    explicit Advertiser(const std::string& hci_iface);
    void setPacketGenerator(std::function<std::vector<uint8_t>()> gen);
    void start(uint32_t interval_ms = 250);
    void stop();
private:
    void advertiseLoop(uint32_t interval_ms);
    bool sendAdvPacket(const std::vector<uint8_t>& data);
    bool set_advertising_data(int sock, const uint8_t* data, size_t len);
};
