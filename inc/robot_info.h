#pragma once
#include "adv_types.h"
#include <map>
#include <vector>
#include <chrono>
#include <mutex>

/**
 * BLE 수신 광고 파싱 후 병합, 최근 상태 추출용
 */
using Timestamp = std::chrono::steady_clock::time_point;

struct MergedRobotAdv {
    RobotInfoTaskAdv info;
    LocalGlobalPlanAdv local_plan, global_plan;
    Timestamp last_local_time, last_global_time, last_info_time;
};

class NearbyRobotManager {
    std::map<uint16_t, MergedRobotAdv> mapNearbyRobot;
    mutable std::mutex mu;
public:
    void updateInfo(const RobotInfoTaskAdv& inf, Timestamp ts);
    void updateLocal(const LocalGlobalPlanAdv& l, Timestamp ts);
    void updateGlobal(const LocalGlobalPlanAdv& g, Timestamp ts);
    std::vector<RobotFullState> getRecent(double sec = 5.0) const;
};
