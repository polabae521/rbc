#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>

class Listener {
    std::string hci_iface_;
    std::thread scan_thread_;
    std::atomic<bool> running_{false};
    std::function<void(const std::vector<uint8_t>&, const std::string&)> on_adv_recv_;
public:
    explicit Listener(const std::string& hci_iface);
    void setCallback(std::function<void(const std::vector<uint8_t>&, const std::string&)> cb);
    void start();
    void stop();
private:
    void scanLoop();
};
