#include "listener.h"
#include <glog/logging.h>
#include <unistd.h>
#include <iostream>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <endian.h>

Listener::Listener(const std::string &hci_iface) : hci_iface_(hci_iface) {}

void Listener::setCallback(std::function<void(const std::vector<uint8_t> &, const std::string &)> cb)
{
  on_adv_recv_ = std::move(cb);
}

void Listener::scanLoop()
{
  /**
   * @brief 광고 수신 루프. read 대신 select+timeout, running_ false 되면 즉시 탈출.
   * 블로킹 문제 예방하고, 소켓도 종료 보장.
   */
  int dev_id = hci_devid(hci_iface_.c_str());
  if (dev_id < 0)
  {
    LOG(ERROR) << "[BLE-LISTEN] No hci device: " << hci_iface_;
    return;
  }
  int sock = hci_open_dev(dev_id);
  if (sock < 0)
  {
    LOG(ERROR) << "[BLE-LISTEN] Open failed: " << hci_iface_;
    return;
  }

  uint8_t scan_type = 0x01;        // active scan
  uint16_t interval = htobs(0x10); // 스캔인터벌
  uint16_t window = htobs(0x10);   // 스캔윈도우
  uint8_t own_addr_type = 0x00;    // public address
  uint8_t filter_policy = 0x00;    // 기본 필터 정책
  int timeout = 1000;              // 1초

  // 함수 호출: 각 인자를 개별로 전달
  hci_le_set_scan_parameters(sock, scan_type, interval, window, own_addr_type, filter_policy, timeout);
  hci_le_set_scan_enable(sock, 0x01, 1, 1000);

  unsigned char buf[HCI_MAX_EVENT_SIZE];
  while (running_)
  {
    // === select로 read 블로킹 해제 및 안전종료 체크 ===
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000; // 0.5초
    int ready = select(sock + 1, &fds, nullptr, nullptr, &timeout);
    if (!running_)
      break; // 종료 신호 즉시 반영
    if (ready > 0 && FD_ISSET(sock, &fds))
    {
      int len = read(sock, buf, sizeof(buf));
      if (len > 0 && on_adv_recv_)
      {
        LOG(INFO) << "[BLE-LISTEN] Packet read: " << len << " bytes";
        // ...패킷 처리
        std::vector<uint8_t> advData(buf, buf + len);
        std::string mac_addr = "xx:xx:xx:xx:xx:xx";
        on_adv_recv_(advData, mac_addr);
      }
    }
  }
  // 안전한 스캔 stop 및 소켓 close
  hci_le_set_scan_enable(sock, 0x00, 0, 1000);
  close(sock);
  LOG(INFO) << "[BLE-LISTEN] scanLoop safely exited. hci=" << hci_iface_;
}

void Listener::start()
{
  running_ = true;
  scan_thread_ = std::thread([this]()
                             { scanLoop(); });
  LOG(INFO) << "[BLE-LISTEN] Started on " << hci_iface_;
}
void Listener::stop()
{
  running_ = false;
  if (scan_thread_.joinable())
    scan_thread_.join();
  LOG(INFO) << "[BLE-LISTEN] Stopped on " << hci_iface_;
}
