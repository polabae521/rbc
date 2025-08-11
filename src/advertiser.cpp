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
  uint32_t elapsed = 0;
  const uint32_t check_step = 50; // ms 단위 체크 (50ms)
  while (running_)
  {
    if (!packet_generator_)
    {
      for (elapsed = 0; running_ && elapsed < 200; elapsed += check_step)
        std::this_thread::sleep_for(std::chrono::milliseconds(check_step));
      continue;
    }
    auto data = packet_generator_();
    if (!data.empty())
    {
      sendAdvPacket(data);
    }
    // 긴 광고 주기도 짧게 쪼개서 종료 즉시 break
    for (elapsed = 0; running_ && elapsed < interval_ms; elapsed += check_step)
      std::this_thread::sleep_for(std::chrono::milliseconds(check_step));
  }
  // 광고 종료, 리소스 해제 로그
  LOG(INFO) << "[BLE-ADV] advertiseLoop safely exited. hci=" << hci_iface_;
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
    LOG(WARNING) << "[BLE-ADV] Open failed: " << hci_iface_;
    return false;
  }
  le_set_advertising_data_cp adv_cp{};
  adv_cp.length = std::min<size_t>(data.size(), sizeof(adv_cp.data));
  memcpy(adv_cp.data, data.data(), adv_cp.length);
  bool rc = set_advertising_data(sock, adv_cp.data, adv_cp.length);
  close(sock);
  if (rc)
  {
    LOG(INFO) << "[BLE-ADV] Advertising data sent successfully.";
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
