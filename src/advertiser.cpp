#include "advertiser.h"
#include <glog/logging.h>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <endian.h>
#include <sstream> // Added for std::stringstream
#include <iomanip> // Added for std::setw and std::setfill
#include <cstring>

Advertiser::Advertiser(const std::string &hci_iface) : hci_iface_(hci_iface) {}

void Advertiser::setPacketGenerator(std::function<std::vector<uint8_t>()> gen) { packet_generator_ = std::move(gen); }

void Advertiser::start(uint32_t interval_ms)
{
  running_ = true;
  adv_thread_ = std::thread([=]
                            { advertiseLoop(interval_ms); });
  LOG(INFO) << "[BLE-ADV] Started on " << hci_iface_;
}
void Advertiser::stop()
{
  running_ = false;
  if (adv_thread_.joinable())
    adv_thread_.join();
  LOG(INFO) << "[BLE-ADV] Stopped on " << hci_iface_;
}

void Advertiser::advertiseLoop(uint32_t interval_ms)
{
  /**
   * @brief 광고 송신 루프. running_ 이 false 되면 즉시 break. sleep은 짧게 쉰 뒤 반복 체크로 안전종료 보장.
   */
  
  // 먼저 BLE 어댑터를 활성화하고 광고를 시작
  if (!enableBleAdvertising()) {
    LOG(ERROR) << "[BLE-ADV] Failed to enable BLE advertising";
    return;
  }
  
  uint32_t elapsed = 0;
  const uint32_t check_step = 50; // ms 단위 체크 (50ms)
  int packet_count = 0;
  
  while (running_)
  {
    if (!packet_generator_)
    {
      LOG(WARNING) << "[BLE-ADV] No packet generator set, waiting...";
      for (elapsed = 0; running_ && elapsed < 200; elapsed += check_step)
        std::this_thread::sleep_for(std::chrono::milliseconds(check_step));
      continue;
    }
    auto data = packet_generator_();
    if (!data.empty())
    {
      LOG(INFO) << "[BLE-ADV] Sending packet #" << ++packet_count << " size=" << data.size() << " bytes";
      if (sendAdvPacket(data)) {
        LOG(INFO) << "[BLE-ADV] Packet sent successfully";
      } else {
        LOG(ERROR) << "[BLE-ADV] Failed to send packet";
      }
    }
    // 긴 광고 주기도 짧게 쪼개서 종료 즉시 break
    for (elapsed = 0; running_ && elapsed < interval_ms; elapsed += check_step)
      std::this_thread::sleep_for(std::chrono::milliseconds(check_step));
  }
  
  // 광고 종료 시 비활성화
  disableBleAdvertising();
  
  // 광고 종료, 리소스 해제 로그
  LOG(INFO) << "[BLE-ADV] advertiseLoop safely exited. hci=" << hci_iface_ << ", total packets sent=" << packet_count;
}

bool Advertiser::enableBleAdvertising()
{
  int dev_id = hci_devid(hci_iface_.c_str());
  if (dev_id < 0) {
    LOG(ERROR) << "[BLE-ADV] No hci device: " << hci_iface_;
    return false;
  }
  
  int sock = hci_open_dev(dev_id);
  if (sock < 0) {
    LOG(ERROR) << "[BLE-ADV] Failed to open device: " << hci_iface_ << " error=" << strerror(errno);
    return false;
  }
  
  // 1. LE 광고 파라미터 설정
  le_set_advertising_parameters_cp adv_params{};
  adv_params.min_interval = htobs(0x0800); // 1.28 seconds
  adv_params.max_interval = htobs(0x0800); // 1.28 seconds
  adv_params.advtype = 0x03; // ADV_NONCONN_IND - Non-connectable undirected advertising
  adv_params.own_bdaddr_type = 0x00; // Public Device Address
  adv_params.direct_bdaddr_type = 0x00;
  memset(adv_params.direct_bdaddr.b, 0, 6);
  adv_params.chan_map = 0x07; // All channels
  adv_params.filter = 0x00; // No filter
  
  struct hci_request rq{};
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
  rq.cparam = &adv_params;
  rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
  
  if (hci_send_req(sock, &rq, 1000) < 0) {
    LOG(ERROR) << "[BLE-ADV] Failed to set advertising parameters: " << strerror(errno);
    close(sock);
    return false;
  }
  LOG(INFO) << "[BLE-ADV] Advertising parameters set successfully";
  
  // 2. LE 광고 활성화
  le_set_advertise_enable_cp enable_cp{};
  enable_cp.enable = 0x01; // Enable advertising
  
  memset(&rq, 0, sizeof(rq));
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
  rq.cparam = &enable_cp;
  rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
  
  if (hci_send_req(sock, &rq, 1000) < 0) {
    LOG(ERROR) << "[BLE-ADV] Failed to enable advertising: " << strerror(errno);
    close(sock);
    return false;
  }
  
  LOG(INFO) << "[BLE-ADV] BLE advertising enabled successfully on " << hci_iface_;
  
  // 디바이스 정보 출력
  bdaddr_t bdaddr;
  if (hci_devba(dev_id, &bdaddr) >= 0) {
    char addr_str[18];
    ba2str(&bdaddr, addr_str);
    LOG(INFO) << "[BLE-ADV] Device MAC address: " << addr_str;
  }
  
  close(sock);
  return true;
}

void Advertiser::disableBleAdvertising()
{
  int dev_id = hci_devid(hci_iface_.c_str());
  if (dev_id < 0) {
    return;
  }
  
  int sock = hci_open_dev(dev_id);
  if (sock < 0) {
    return;
  }
  
  // LE 광고 비활성화
  le_set_advertise_enable_cp enable_cp{};
  enable_cp.enable = 0x00; // Disable advertising
  
  struct hci_request rq{};
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
  rq.cparam = &enable_cp;
  rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
  
  hci_send_req(sock, &rq, 1000);
  close(sock);
  
  LOG(INFO) << "[BLE-ADV] BLE advertising disabled on " << hci_iface_;
}

bool Advertiser::sendAdvPacket(const std::vector<uint8_t> &data)
{
  int dev_id = hci_devid(hci_iface_.c_str());
  if (dev_id < 0)
  {
    LOG(WARNING) << "[BLE-ADV] No hci device: " << hci_iface_;
    return false;
  }
  int sock = hci_open_dev(dev_id);
  if (sock < 0)
  {
    LOG(WARNING) << "[BLE-ADV] Open failed: " << hci_iface_ << " error=" << strerror(errno);
    return false;
  }
  
  // 데이터 패킷 내용 로그
  std::stringstream hex_dump;
  hex_dump << std::hex;
  for (size_t i = 0; i < std::min(data.size(), size_t(16)); ++i) {
    hex_dump << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
  }
  LOG(INFO) << "[BLE-ADV] Packet data (first 16 bytes): " << hex_dump.str();
  
  le_set_advertising_data_cp adv_cp{};
  adv_cp.length = std::min<size_t>(data.size(), sizeof(adv_cp.data));
  memcpy(adv_cp.data, data.data(), adv_cp.length);
  bool rc = set_advertising_data(sock, adv_cp.data, adv_cp.length);
  close(sock);
  if (rc)
  {
    LOG(INFO) << "[BLE-ADV] Advertising data sent successfully. Size=" << adv_cp.length;
  }
  else
  {
    LOG(ERROR) << "[BLE-ADV] Advertising data send error: " << strerror(errno);
  }
  return rc;
}


bool Advertiser::set_advertising_data(int sock, const uint8_t *data, size_t len)
{
  struct hci_request rq;
  le_set_advertising_data_cp adv_cp{};
  adv_cp.length = std::min(len, sizeof(adv_cp.data));
  memcpy(adv_cp.data, data, adv_cp.length);

  memset(&rq, 0, sizeof(rq));
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISING_DATA;
  rq.cparam = &adv_cp;
  rq.clen = LE_SET_ADVERTISING_DATA_CP_SIZE;
  rq.rparam = NULL;
  rq.rlen = 0;

  int err = hci_send_req(sock, &rq, 1000); // hci_send_req도 hci_lib.h에 반드시 정의
  return (err >= 0);
}
