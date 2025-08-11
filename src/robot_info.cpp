// robot_info.cpp
#include "robot_info.h"
#include <chrono>

void NearbyRobotManager::updateInfo(const RobotInfoTaskAdv& inf, Timestamp ts) {
    std::lock_guard<std::mutex> lock(mu);
    auto& merged = mapNearbyRobot[inf.robot_id];
    merged.info = inf;
    merged.last_info_time = ts;
}
void NearbyRobotManager::updateLocal(const LocalGlobalPlanAdv& l, Timestamp ts) {
    std::lock_guard<std::mutex> lock(mu);
    auto& merged = mapNearbyRobot[l.robot_id];
    merged.local_plan = l;
    merged.last_local_time = ts;
}
void NearbyRobotManager::updateGlobal(const LocalGlobalPlanAdv& g, Timestamp ts) {
    std::lock_guard<std::mutex> lock(mu);
    auto& merged = mapNearbyRobot[g.robot_id];
    merged.global_plan = g;
    merged.last_global_time = ts;
}
std::vector<RobotFullState> NearbyRobotManager::getRecent(double sec) const {
    std::vector<RobotFullState> result;
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mu);
    for (const auto& [rid, merged] : mapNearbyRobot) {
        double diff_local  = std::chrono::duration<double>(now - merged.last_local_time).count();
        double diff_info   = std::chrono::duration<double>(now - merged.last_info_time).count();
        if (diff_local <= sec || diff_info <= sec) {
            RobotFullState fs;
            fs.robot_id      = merged.info.robot_id;
            fs.max_speed     = merged.info.max_speed;
            fs.width         = merged.info.width;
            fs.length        = merged.info.length;
            fs.goal_vertex_id= merged.info.goal_vertex_id;
            fs.utc_task_time = merged.info.utc_task_time;
            for (int i = 0; i < 9; ++i) fs.vertices.push_back(merged.local_plan.vertex_id[i]);
            result.push_back(fs);
        }
    }
    return result; // 값 복사본
}
