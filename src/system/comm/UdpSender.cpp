#include "UdpSender.h"

bool UdpSender::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000) {
        delay(100);
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("UDP: WiFi connect failed");
        _ready = false;
        return false;
    }
    _target = IPAddress(UDP_TARGET_IP0, UDP_TARGET_IP1, UDP_TARGET_IP2, UDP_TARGET_IP3);
    _udp.begin(0); // ephemeral local port
    _ready = true;
    Serial.printf("UDP: ready, target=%s:%u\n", _target.toString().c_str(), _port);
    return true;
}

bool UdpSender::sendImuPacket(const uint8_t* buf, size_t n, IPAddress target, uint16_t port) {
    if (!_ready) return false;
    _udp.beginPacket(target, port);
    _udp.write(buf, n);
    _udp.endPacket();
    return true;
}

bool UdpSender::sendControl(
    float ax, float ay, float az,
    float gx, float gy, float gz,
    uint8_t tempByte,
    const uint16_t* servoPos8,
    const uint16_t* servoOff8,
    uint16_t seq,
    bool includeImu) {
    if (!_ready) return false;
    
    // IMU出力無効時は0を送信
    float _ax = includeImu ? ax : 0.0f;
    float _ay = includeImu ? ay : 0.0f;
    float _az = includeImu ? az : 0.0f;
    float _gx = includeImu ? gx : 0.0f;
    float _gy = includeImu ? gy : 0.0f;
    float _gz = includeImu ? gz : 0.0f;
    uint8_t _t8 = includeImu ? tempByte : 0;
    
    uint8_t buf[80];
    size_t n = CommProtocol::buildControlPacket(buf, sizeof(buf),
        _ax, _ay, _az, _gx, _gy, _gz, _t8,
        servoPos8, servoOff8, seq, false);
    if (n == 0) return false;
    _udp.beginPacket(_target, _port);
    _udp.write(buf, n);
    _udp.endPacket();
    return true;
}
