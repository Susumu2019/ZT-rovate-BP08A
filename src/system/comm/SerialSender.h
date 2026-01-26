#pragma once
#include <Arduino.h>
#include "CommProtocol.h"
#include "config.h"

class SerialSender {
public:
    SerialSender() : _ready(false) {}
    bool begin();
    bool isReady() const { return _ready; }

    // バイナリ送信
    bool sendControl(
        float ax, float ay, float az,
        float gx, float gy, float gz,
        uint8_t tempByte,
        const uint16_t* servoPos8,
        const uint16_t* servoOff8,
        uint16_t seq,
        bool includeImu = true);  // IMUデータを含むか

    // テキスト（JSON）送信
    bool sendControlText(
        float ax, float ay, float az,
        float gx, float gy, float gz,
        uint8_t tempByte,
        const uint16_t* servoPos8,
        const uint16_t* servoOff8,
        uint16_t seq,
        bool includeImu = true);  // IMUデータを含むか

    // テキストコマンド受信・処理
    // 戻り値: コマンドを受信して処理した場合true
    bool processTextCommand(uint16_t* servoPos8, uint16_t* servoOff8);

    // バイナリコマンド受信・処理
    // 戻り値: コマンドを受信して処理した場合true
    bool processBinaryCommand(uint16_t* servoPos8, uint16_t* servoOff8);

private:
    bool _ready;
    String _rxBuffer;  // テキスト受信バッファ
    uint8_t _rxBinBuf[80];  // バイナリ受信バッファ
    size_t _rxBinIdx = 0;
    static constexpr size_t MAX_BUFFER_SIZE = 1024;
};
