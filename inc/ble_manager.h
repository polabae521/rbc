#pragma once
#include "advertiser.h"
#include "listener.h"
#include "adv_types.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <map>

class BleManager
{
  std::string ble_iface_;
  Advertiser advertiser_;
  Listener listener_;
  mutable std::mutex mu_info_;
  RobotInfoTaskAdv my_info_adv_;
  std::atomic<bool> listen_echo_{false};
  std::map<std::string, RobotFullState> neighbors_;

private:
  bool enable_adv_;

public:
  BleManager(bool listen_echo, std::string ble_iface, bool enable_adv);

  void setAdvertiseInfo(const RobotFullState &);
  std::vector<RobotFullState> getNearbyRobotInfo() const;
  void start();
  void stop();

private:
  std::vector<uint8_t> genRobotInfoAdv() const;
  void mergeRobotInfo(const std::vector<uint8_t> &adv, const std::string &mac);
  uint16_t calc_crc(const void *data, size_t nbyte) const;
};
