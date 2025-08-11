#!/bin/bash

echo "========================================="
echo "BLE Adapter Status Check"
echo "========================================="

# 1. Bluetooth 서비스 상태 확인
echo -e "\n[1] Bluetooth Service Status:"
sudo systemctl status bluetooth --no-pager | head -10

# 2. HCI 디바이스 확인
echo -e "\n[2] HCI Devices:"
hciconfig -a

# 3. BLE 어댑터 상태 확인
echo -e "\n[3] Bluetooth Controller Info:"
sudo bluetoothctl show

# 4. LE 광고 상태 확인
echo -e "\n[4] LE Advertising Status:"
sudo hciconfig hci0 leadv
sudo hciconfig hci0

# 5. BLE 스캔 가능 여부 확인
echo -e "\n[5] LE Scan Test (5 seconds):"
sudo timeout 5 hcitool lescan

# 6. 현재 LE 광고 파라미터 확인
echo -e "\n[6] Current LE Advertising Parameters:"
sudo hcitool -i hci0 cmd 0x08 0x0008

# 7. 커널 모듈 확인
echo -e "\n[7] Bluetooth Kernel Modules:"
lsmod | grep -i bluetooth

echo -e "\n========================================="
echo "Check Complete"
echo "========================================="