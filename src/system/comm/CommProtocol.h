#pragma once
#include <stdint.h>
#include <stddef.h>

// 簡易プロトコル定義
// [SYNC(2) AA 55][VER(1)=1][TYPE(1)=1][SEQ(2)][LEN(2)=57][PAYLOAD(57)][CRC16(2)][(optional ETX 1)]

namespace CommProtocol {

static constexpr uint8_t SYNC0 = 0xAA;
static constexpr uint8_t SYNC1 = 0x55;
static constexpr uint8_t VERSION = 0x01;
static constexpr uint8_t TYPE_CONTROL = 0x01;
static constexpr uint16_t PAYLOAD_LEN = 57; // IMU(25) + servo pos(16) + servo off(16)

// CRC16-CCITT (0x1021), init 0xFFFF
uint16_t crc16_ccitt(const uint8_t* data, size_t len);

// 制御パケット生成（out に書き込み）。
// addEtx=true の場合、末尾に 0x7E を追加（UART用フレーミング）。
// 戻り値: 生成されたバイト数（ヘッダ+ペイロード+CRC(+ETX)）
size_t buildControlPacket(
    uint8_t* out, size_t outMax,
    float ax, float ay, float az,
    float gx, float gy, float gz,
    uint8_t tempByte,
    const uint16_t* servoPos8, // 長さ8
    const uint16_t* servoOff8, // 長さ8
    uint16_t seq,
    bool addEtx);

} // namespace CommProtocol
