# BLE 광고 송신 문제 해결 가이드

## 문제 상황
노트북에서 BLE 광고를 송신하려 하지만 핸드폰의 BLE 스캐너에서 MAC 주소가 보이지 않는 문제

## 수정 사항

### 1. advertiser.cpp 개선 사항
- ✅ BLE 광고 활성화 코드 추가 (`enableBleAdvertising()`)
- ✅ 광고 파라미터 설정 (interval, type, channel map)
- ✅ 상세한 디버깅 로그 추가
- ✅ MAC 주소 출력 기능 추가
- ✅ 패킷 데이터 HEX 덤프 기능 추가

### 2. 주요 코드 변경 내용

#### advertiser.cpp에 추가된 기능:
```cpp
// BLE 광고 활성화
bool enableBleAdvertising() {
    // 1. LE 광고 파라미터 설정
    // 2. LE 광고 활성화 (OCF_LE_SET_ADVERTISE_ENABLE)
    // 3. 디바이스 MAC 주소 출력
}

// BLE 광고 비활성화
void disableBleAdvertising() {
    // LE 광고 비활성화
}
```

## 실제 하드웨어에서 확인해야 할 사항

### 1. BLE 어댑터 상태 확인
```bash
# HCI 디바이스 확인
sudo hciconfig -a

# BLE 어댑터가 UP 상태인지 확인
sudo hciconfig hci0 up

# LE 광고 활성화
sudo hciconfig hci0 leadv

# BLE 스캔 가능 여부 테스트
sudo hcitool lescan
```

### 2. 프로그램 실행 방법
```bash
# 빌드
cd /workspace/build
cmake .. && make -j5

# BLE 송신 모드로 실행 (root 권한 필요)
sudo /workspace/bin/robot_ble_communicator --role=send --ble_iface=hci0

# 더 자세한 로그를 보려면
sudo GLOG_logtostderr=1 GLOG_v=2 /workspace/bin/robot_ble_communicator --role=send --ble_iface=hci0
```

### 3. 문제 해결 체크리스트

#### ✅ 하드웨어 확인
- [ ] Bluetooth 어댑터가 BLE를 지원하는가? (Bluetooth 4.0 이상)
- [ ] 어댑터가 활성화되어 있는가? (`hciconfig hci0 up`)
- [ ] rfkill로 블록되어 있지 않은가? (`rfkill list`)

#### ✅ 소프트웨어 확인
- [ ] BlueZ가 설치되어 있는가? (`apt install bluez`)
- [ ] 프로그램을 root 권한으로 실행하는가?
- [ ] 올바른 HCI 인터페이스를 사용하는가? (hci0, hci1 등)

#### ✅ BLE 광고 설정 확인
- [ ] 광고 타입이 올바른가? (ADV_NONCONN_IND = 0x03)
- [ ] 광고 간격이 적절한가? (1.28초)
- [ ] 모든 채널에서 광고하는가? (channel map = 0x07)

### 4. 디버깅 팁

#### 로그 확인
프로그램 실행 시 다음과 같은 로그가 출력되어야 합니다:
```
[BLE-ADV] Advertising parameters set successfully
[BLE-ADV] BLE advertising enabled successfully on hci0
[BLE-ADV] Device MAC address: XX:XX:XX:XX:XX:XX
[BLE-ADV] Sending packet #1 size=31 bytes
[BLE-ADV] Packet data (first 16 bytes): 02 01 06 ...
[BLE-ADV] Advertising data sent successfully. Size=31
```

#### 핸드폰에서 확인
1. nRF Connect, LightBlue 등의 BLE 스캐너 앱 사용
2. 스캔 필터 해제 (모든 디바이스 표시)
3. RSSI 임계값 낮추기
4. 광고 패킷의 RAW 데이터 확인

### 5. 추가 테스트 명령어

```bash
# BLE 광고 수동 테스트
sudo hcitool -i hci0 cmd 0x08 0x0008 1E 02 01 06 1A FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

# 광고 활성화
sudo hcitool -i hci0 cmd 0x08 0x000A 01

# 현재 광고 상태 확인
sudo hcitool -i hci0 cmd 0x08 0x000B
```

## 예상 문제 원인

1. **HCI 디바이스 문제**: hci0이 존재하지 않거나 비활성화 상태
2. **권한 문제**: root 권한 없이 실행
3. **광고 미활성화**: LE 광고가 활성화되지 않음
4. **패킷 구조 문제**: 광고 패킷 데이터가 BLE 표준을 따르지 않음
5. **하드웨어 호환성**: BLE를 지원하지 않는 Bluetooth 어댑터

## 해결 방법

1. **어댑터 확인 및 활성화**
```bash
sudo systemctl start bluetooth
sudo hciconfig hci0 up
sudo hciconfig hci0 leadv
```

2. **프로그램 재실행**
```bash
sudo /workspace/bin/robot_ble_communicator --role=send --ble_iface=hci0
```

3. **다른 인터페이스 시도**
```bash
# hci1 사용
sudo /workspace/bin/robot_ble_communicator --role=send --ble_iface=hci1
```

4. **BlueZ 재시작**
```bash
sudo systemctl restart bluetooth
```

## 참고 자료
- BlueZ Documentation: http://www.bluez.org/
- BLE Advertising: https://www.bluetooth.com/specifications/
- HCI Commands: https://www.bluetooth.com/specifications/assigned-numbers/