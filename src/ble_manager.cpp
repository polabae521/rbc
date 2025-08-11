#include "ble_manager.h"
#include <glog/logging.h>
#include <cstring>
#include <iostream>
#include <algorithm>

BleManager::BleManager(bool listen_echo, std::string ble_iface, bool enable_adv)
    : listen_echo_(listen_echo), ble_iface_(ble_iface), enable_adv_(enable_adv),
      advertiser_(ble_iface), listener_(ble_iface)
{
    listener_.setCallback([this](const std::vector<uint8_t>& adv, const std::string& mac){
        this->mergeRobotInfo(adv, mac);
    });
}

void BleManager::setAdvertiseInfo(const RobotFullState& fs) {
    std::lock_guard<std::mutex> lock(mu_info_);
    my_info_adv_.robot_id = fs.robot_id;
    my_info_adv_.max_speed = fs.max_speed;
    my_info_adv_.width = fs.width;
    my_info_adv_.length = fs.length;
    my_info_adv_.goal_vertex_id = fs.goal_vertex_id;
    my_info_adv_.utc_task_time = fs.utc_task_time;
    // MAC/plan 등은 실제 환경에 맞게 추가
    my_info_adv_.crc = calc_crc(&my_info_adv_, sizeof(my_info_adv_) - sizeof(my_info_adv_.crc));
}

void BleManager::start() {
    if (enable_adv_) {
        advertiser_.setPacketGenerator([this]{ return genRobotInfoAdv(); });
        advertiser_.start(250);
    }
    listener_.start();
}

void BleManager::stop() {
    if (enable_adv_) advertiser_.stop();
    listener_.stop();
}

std::vector<uint8_t> BleManager::genRobotInfoAdv() const {
    std::lock_guard<std::mutex> lock(mu_info_);
    std::vector<uint8_t> packet(sizeof(my_info_adv_));
    std::memcpy(packet.data(), &my_info_adv_, sizeof(my_info_adv_));
    return packet;
}

void BleManager::mergeRobotInfo(const std::vector<uint8_t>& adv, const std::string& mac) {
    /**
     * @brief BLE 광고 패킷 수신 시 Echo 로그 및 파싱 성공 여부 표시.
     */
    if (listen_echo_) {
        LOG(INFO) << "[Echo] RX from " << mac << " | Raw: "
                  << std::hex << adv.size() << " bytes";
        // HEX dump 및 파싱 결과 추가
        for (size_t i = 0; i < adv.size(); ++i)
            LOG_IF(INFO, i < 16) << std::hex << int(adv[i]) << " ";
        LOG(INFO) << std::dec;
    }
    // 실제 파싱, 필터, robot_id 비교 etc.
}

std::vector<RobotFullState> BleManager::getNearbyRobotInfo() const {
    std::lock_guard<std::mutex> lock(mu_info_);
    std::vector<RobotFullState> out;
    for (const auto& kv : neighbors_) out.push_back(kv.second);
    return out;
}
uint16_t BleManager::calc_crc(const void* data, size_t nbyte) const {
    uint16_t crc = 0;
    const uint8_t* p = (const uint8_t*)data;
    for(size_t i=0;i<nbyte;++i) crc = (crc<<1)^p[i];
    return crc;
}
