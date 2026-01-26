/**
 * MPU6886 IMUセンサードライバ実装
 */

#include "MPU6886.h"

MPU6886::MPU6886() 
  : wire_(nullptr), i2cAddress_(MPU6886_ADDRESS), deviceID_(0),
    accelScale_(AFS_8G), gyroScale_(GFS_2000DPS),
    accelRes_(0.0f), gyroRes_(0.0f) {
}

int MPU6886::begin(TwoWire* wire, uint8_t address) {
  wire_ = wire;
  i2cAddress_ = address;

  // デバイスIDを読み取る
  uint8_t whoami;
  readBytes(MPU6886_WHOAMI, 1, &whoami);
  deviceID_ = whoami;
  delay(1);

  // WHOAMI値をチェック（MPU6886/9の正常なIDは0x19または0x70）
  if (whoami != 0x19 && whoami != 0x70) {
    return -1;  // デバイスが見つからない
  }

  // デバイスをリセット
  writeByte(MPU6886_PWR_MGMT_1, 0x00);
  delay(10);

  // デバイスリセット
  writeByte(MPU6886_PWR_MGMT_1, 0x80);
  delay(10);

  // ウェイクアップして内部オシレータを使用
  writeByte(MPU6886_PWR_MGMT_1, 0x01);
  delay(10);

  // 加速度計を設定 (±8g)
  writeByte(MPU6886_ACCEL_CONFIG, 0x10);
  delay(1);

  // ジャイロスコープを設定 (±2000dps)
  writeByte(MPU6886_GYRO_CONFIG, 0x18);
  delay(1);

  // サンプルレート分周器を設定 (1kHz / (1 + div))
  writeByte(MPU6886_CONFIG, 0x01);  // 1kHz出力
  delay(1);

  writeByte(MPU6886_SMPLRT_DIV, 0x01);  // 500Hz
  delay(1);

  // 割り込みを無効化
  writeByte(MPU6886_INT_ENABLE, 0x00);
  delay(1);

  // 加速度計LPFを設定
  writeByte(MPU6886_ACCEL_CONFIG2, 0x00);
  delay(1);

  // ユーザー制御機能を無効化
  writeByte(MPU6886_USER_CTRL, 0x00);
  delay(1);

  // FIFOを無効化
  writeByte(MPU6886_FIFO_EN, 0x00);
  delay(1);

  // 割り込みピンを設定
  writeByte(MPU6886_INT_PIN_CFG, 0x22);
  delay(1);

  // データレディ割り込みを有効化
  writeByte(MPU6886_INT_ENABLE, 0x01);
  delay(10);

  // スケール解像度を更新
  setAccelScale(accelScale_);
  setGyroScale(gyroScale_);

  return 0;
}

void MPU6886::readAccelADC(int16_t* ax, int16_t* ay, int16_t* az) {
  uint8_t buf[6];
  readBytes(MPU6886_ACCEL_XOUT_H, 6, buf);
  *ax = ((int16_t)buf[0] << 8) | buf[1];
  *ay = ((int16_t)buf[2] << 8) | buf[3];
  *az = ((int16_t)buf[4] << 8) | buf[5];
}

void MPU6886::readGyroADC(int16_t* gx, int16_t* gy, int16_t* gz) {
  uint8_t buf[6];
  readBytes(MPU6886_GYRO_XOUT_H, 6, buf);
  *gx = ((int16_t)buf[0] << 8) | buf[1];
  *gy = ((int16_t)buf[2] << 8) | buf[3];
  *gz = ((int16_t)buf[4] << 8) | buf[5];
}

void MPU6886::readTempADC(int16_t* temp) {
  uint8_t buf[2];
  readBytes(MPU6886_TEMP_OUT_H, 2, buf);
  *temp = ((int16_t)buf[0] << 8) | buf[1];
}

void MPU6886::readAccel(float* ax, float* ay, float* az) {
  int16_t rawX, rawY, rawZ;
  readAccelADC(&rawX, &rawY, &rawZ);
  *ax = (float)rawX * accelRes_;
  *ay = (float)rawY * accelRes_;
  *az = (float)rawZ * accelRes_;
}

void MPU6886::readGyro(float* gx, float* gy, float* gz) {
  int16_t rawX, rawY, rawZ;
  readGyroADC(&rawX, &rawY, &rawZ);
  *gx = (float)rawX * gyroRes_;
  *gy = (float)rawY * gyroRes_;
  *gz = (float)rawZ * gyroRes_;
}

void MPU6886::readTemp(float* temp) {
  int16_t rawTemp;
  readTempADC(&rawTemp);
  *temp = (float)rawTemp / 326.8f + 25.0f;
}

void MPU6886::updateGyroRes() {
  switch (gyroScale_) {
    case GFS_250DPS:  gyroRes_ = 250.0f / 32768.0f; break;
    case GFS_500DPS:  gyroRes_ = 500.0f / 32768.0f; break;
    case GFS_1000DPS: gyroRes_ = 1000.0f / 32768.0f; break;
    case GFS_2000DPS: gyroRes_ = 2000.0f / 32768.0f; break;
  }
}

void MPU6886::updateAccelRes() {
  switch (accelScale_) {
    case AFS_2G:  accelRes_ = 2.0f / 32768.0f; break;
    case AFS_4G:  accelRes_ = 4.0f / 32768.0f; break;
    case AFS_8G:  accelRes_ = 8.0f / 32768.0f; break;
    case AFS_16G: accelRes_ = 16.0f / 32768.0f; break;
  }
}

void MPU6886::setGyroScale(GyroScale scale) {
  uint8_t config = (scale << 3);
  writeByte(MPU6886_GYRO_CONFIG, config);
  delay(10);
  gyroScale_ = scale;
  updateGyroRes();
}

void MPU6886::setAccelScale(AccelScale scale) {
  uint8_t config = (scale << 3);
  writeByte(MPU6886_ACCEL_CONFIG, config);
  delay(10);
  accelScale_ = scale;
  updateAccelRes();
}

void MPU6886::setGyroOffset(uint16_t x, uint16_t y, uint16_t z) {
  uint8_t buf[6];
  buf[0] = x >> 8;
  buf[1] = x & 0xFF;
  buf[2] = y >> 8;
  buf[3] = y & 0xFF;
  buf[4] = z >> 8;
  buf[5] = z & 0xFF;
  writeBytes(MPU6886_GYRO_OFFSET, 6, buf);
}

void MPU6886::setFIFOEnabled(bool enable) {
  writeByte(MPU6886_FIFO_EN, enable ? 0x18 : 0x00);
  writeByte(MPU6886_USER_CTRL, enable ? 0x40 : 0x00);
}

uint16_t MPU6886::getFIFOCount() {
  uint8_t buf[2];
  readBytes(MPU6886_FIFO_COUNT, 2, buf);
  return ((uint16_t)buf[0] << 8) | buf[1];
}

uint8_t MPU6886::readFIFO() {
  uint8_t data;
  readBytes(MPU6886_FIFO_R_W, 1, &data);
  return data;
}

void MPU6886::readFIFOBuffer(uint8_t* buffer, uint16_t length) {
  uint16_t chunks = length / 210;
  for (uint16_t i = 0; i < chunks; i++) {
    readBytes(MPU6886_FIFO_R_W, 210, &buffer[i * 210]);
  }
  uint16_t remainder = length % 210;
  if (remainder > 0) {
    readBytes(MPU6886_FIFO_R_W, remainder, &buffer[chunks * 210]);
  }
}

void MPU6886::resetFIFO() {
  uint8_t ctrl;
  readBytes(MPU6886_USER_CTRL, 1, &ctrl);
  ctrl |= 0x04;
  writeByte(MPU6886_USER_CTRL, ctrl);
}

void MPU6886::readBytes(uint8_t reg, uint8_t count, uint8_t* buffer) {
  wire_->beginTransmission(i2cAddress_);
  wire_->write(reg);
  wire_->endTransmission(false);
  wire_->requestFrom(i2cAddress_, count);
  for (uint8_t i = 0; i < count && wire_->available(); i++) {
    buffer[i] = wire_->read();
  }
}

void MPU6886::writeBytes(uint8_t reg, uint8_t count, uint8_t* buffer) {
  wire_->beginTransmission(i2cAddress_);
  wire_->write(reg);
  wire_->write(buffer, count);
  wire_->endTransmission();
}

void MPU6886::writeByte(uint8_t reg, uint8_t value) {
  wire_->beginTransmission(i2cAddress_);
  wire_->write(reg);
  wire_->write(value);
  wire_->endTransmission();
}
