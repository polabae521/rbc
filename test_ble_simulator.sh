#!/bin/bash

echo "========================================="
echo "BLE Advertising Simulator Test"
echo "========================================="

# 테스트 모드로 실행 (실제 HCI 디바이스 없이 로그만 출력)
echo -e "\n[1] Testing BLE Advertiser with debug logs..."
cd /workspace/bin

# GLOG 환경 변수 설정 (상세 로그 출력)
export GLOG_logtostderr=1
export GLOG_v=2

echo -e "\n[2] Starting BLE sender (will fail on HCI but show logs)..."
timeout 5 ./robot_ble_communicator --role=send --ble_iface=hci0 --duration=5 2>&1 | tee ble_sender.log

echo -e "\n[3] Log analysis:"
echo "- Advertising attempts:"
grep -c "BLE-ADV" ble_sender.log || echo "0"

echo "- Errors detected:"
grep "ERROR" ble_sender.log || echo "No errors found"

echo "- Warnings detected:"
grep "WARNING" ble_sender.log || echo "No warnings found"

echo -e "\n========================================="
echo "Test Complete"
echo "========================================="