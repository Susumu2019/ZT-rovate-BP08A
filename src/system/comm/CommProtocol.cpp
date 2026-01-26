#include "CommProtocol.h"
#include <string.h>

namespace CommProtocol {

static inline void write_u16le(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

uint16_t crc16_ccitt(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

size_t buildControlPacket(
    uint8_t* out, size_t outMax,
    float ax, float ay, float az,
    float gx, float gy, float gz,
    uint8_t tempByte,
    const uint16_t* servoPos8,
    const uint16_t* servoOff8,
    uint16_t seq,
    bool addEtx) {

    const size_t headerLen = 2 + 1 + 1 + 2 + 2; // SYNC2 + VER + TYPE + SEQ2 + LEN2 = 8
    const size_t payloadLen = PAYLOAD_LEN;      // 57
    const size_t crcLen = 2;
    const size_t etxLen = addEtx ? 1 : 0;      // 0x7E
    const size_t totalLen = headerLen + payloadLen + crcLen + etxLen;

    if (!out || outMax < totalLen) return 0;

    uint8_t* p = out;
    // Header
    *p++ = SYNC0;
    *p++ = SYNC1;
    *p++ = VERSION;
    *p++ = TYPE_CONTROL;
    write_u16le(p, seq); p += 2;
    write_u16le(p, (uint16_t)payloadLen); p += 2;

    // Payload layout:
    // ax, ay, az (float) x3 = 12
    // gx, gy, gz (float) x3 = 12
    // temp (uint8)       = 1
    // servoPos[8] (u16)  = 16
    // servoOff[8] (u16)  = 16

    // IMU accel
    memcpy(p, &ax, sizeof(float)); p += sizeof(float);
    memcpy(p, &ay, sizeof(float)); p += sizeof(float);
    memcpy(p, &az, sizeof(float)); p += sizeof(float);
    // IMU gyro
    memcpy(p, &gx, sizeof(float)); p += sizeof(float);
    memcpy(p, &gy, sizeof(float)); p += sizeof(float);
    memcpy(p, &gz, sizeof(float)); p += sizeof(float);
    // temp
    *p++ = tempByte;
    // servo positions
    for (int i = 0; i < 8; ++i) { write_u16le(p, servoPos8 ? servoPos8[i] : 0); p += 2; }
    // servo offsets
    for (int i = 0; i < 8; ++i) { write_u16le(p, servoOff8 ? servoOff8[i] : 0); p += 2; }

    // CRC over header+payload
    const size_t crcStartLen = headerLen + payloadLen;
    uint16_t crc = crc16_ccitt(out, crcStartLen);
    write_u16le(p, crc); p += 2;

    // optional ETX
    if (addEtx) { *p++ = 0x7E; }

    return (size_t)(p - out);
}

} // namespace CommProtocol
