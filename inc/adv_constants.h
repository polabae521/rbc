#pragma once
#include <cstdint>

namespace Adv {
// BLE 서비스 고유 번호
constexpr uint16_t SERVICE_ID = 0x95A7;
// 광고 타입 구분
enum class AdvType : uint8_t {
    GLOBAL_PLAN  = 0xFC,
    LOCAL_PLAN   = 0xFD,
    ROBOT_INFO   = 0xFE
};
constexpr uint16_t MEANINGLESS_VERTEX = 0xFFFF;
}
