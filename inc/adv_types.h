#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include <array>

namespace Adv {
constexpr uint16_t SERVICE_ID = 0x95A7;
enum class AdvType : uint8_t { GLOBAL_PLAN = 0xFC, LOCAL_PLAN = 0xFD, ROBOT_INFO = 0xFE };
constexpr uint16_t MEANINGLESS_VERTEX = 0xFFFF;
}

struct RobotFullState {
    uint16_t robot_id = 0, max_speed = 0, goal_vertex_id = 0;
    uint8_t width = 0, length = 0;
    uint32_t utc_task_time = 0;
    std::vector<uint16_t> vertices;
    RobotFullState() : robot_id(0), max_speed(0), width(0), length(0), goal_vertex_id(0), utc_task_time(0) {}
};

struct RobotInfoTaskAdv {
    uint16_t service_id = Adv::SERVICE_ID;
    uint8_t adv_type = static_cast<uint8_t>(Adv::AdvType::ROBOT_INFO);
    uint16_t robot_id = 0, max_speed = 0, goal_vertex_id = 0;
    uint8_t width = 0, length = 0;
    uint32_t utc_task_time = 0;
    uint8_t mac[6]{};
    uint16_t crc = 0;
    RobotInfoTaskAdv() { std::memset(mac, 0, sizeof(mac)); }
};

struct LocalGlobalPlanAdv {
    uint8_t adv_type = 0;
    uint16_t robot_id = 0;
    uint16_t vertex_id[9]{};
    uint16_t crc = 0;
    LocalGlobalPlanAdv(Adv::AdvType type=Adv::AdvType::LOCAL_PLAN)
    : adv_type(static_cast<uint8_t>(type)) { std::fill(std::begin(vertex_id), std::end(vertex_id), Adv::MEANINGLESS_VERTEX);}
};
