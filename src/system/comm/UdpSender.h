#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "CommProtocol.h"
#include "config.h"

class UdpSender {
public:
    UdpSender() : _ready(false), _port(UDP_TARGET_PORT) {}
    bool begin();
    bool isReady() const { return _ready; }

    bool sendControl(
        float ax, float ay, float az,
        float gx, float gy, float gz,
        uint8_t tempByte,
        const uint16_t* servoPos8,
        const uint16_t* servoOff8,
        uint16_t seq,
        bool includeImu = true);  // IMUデータを含むか

    // IMUデータ専用UDP送信
    bool sendImuPacket(const uint8_t* buf, size_t n, IPAddress target, uint16_t port);

private:
    WiFiUDP _udp;
    IPAddress _target;
    uint16_t _port;
    bool _ready;
};
