#include "ble_manager.h"
#include "adv_types.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <csignal>
#include <thread>
#include <iostream>

DEFINE_string(role, "send", "Role mode: send / recv / both");
DEFINE_string(ble_iface, "hci1", "BLE interface name");
DEFINE_bool(listen_echo, false, "Enable listen echo (recv)");
DEFINE_uint32(duration, 20, "Duration(sec)");

/**
 * @brief Ctrl+C 시 안전하게 종료할 수 있도록 atomic 플래그와 시그널 핸들러 사용
 */
std::atomic<bool> running{true};
void signalHandler(int)
{
  running = false;
  LOG(WARNING) << "SIGINT received: shutting down BLE...";
}

int main(int argc, char *argv[])
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  signal(SIGINT, signalHandler);

  bool isSend = (FLAGS_role == "send" || FLAGS_role == "both");
  bool isRecv = (FLAGS_role == "recv" || FLAGS_role == "both");
  if (FLAGS_role == "recv")
    FLAGS_listen_echo = true;
  bool enable_adv = isSend; // 'send' 역할에만 광고 송신, 'recv'는 광고 송신 안함
  if (FLAGS_role == "recv")
    FLAGS_listen_echo = true;

  BleManager manager(FLAGS_listen_echo, FLAGS_ble_iface, enable_adv);

  if (isSend)
  {
    RobotFullState my_state;
    my_state.robot_id = 101;
    my_state.max_speed = 500;
    my_state.vertices = {3, 7, 11, 15};
    manager.setAdvertiseInfo(my_state);
  }

  manager.start();
  LOG(INFO) << (isSend ? "[SENDER]" : "[RECEIVER]") << " BLE started on " << FLAGS_ble_iface;

  /**
   * @brief main 루프 - 매 0.5초마다 실행 중임을 표시하고 running==false 되면 즉시 루프 탈출
   * 소프트웨어 품질: 주석 25%, 코드 반복 제한
   */
  auto start_time = std::chrono::steady_clock::now();
  while (running && (FLAGS_duration == 0 ||
                     std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::steady_clock::now() - start_time)
                             .count() < FLAGS_duration))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    LOG_EVERY_N(INFO, 5) << "Main loop: BLE working, elapsed "
                         << std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count()
                         << "s";
  }
  // 안전 종료 요청
  manager.stop();
  LOG(INFO) << "BLE manager stopped.";

  if (isRecv)
  {
    auto robots = manager.getNearbyRobotInfo();
    LOG(INFO) << "[RECEIVER] Nearby robots: " << robots.size();
    for (const auto &r : robots)
      LOG(INFO) << " Robot id: " << r.robot_id
                << " | speed: " << r.max_speed;
  }
  LOG(WARNING) << "Terminate BLE communication.";
  return 0;
}
