/**
 * MPU6886 AHRS実装
 */

#include "MPU6886_AHRS.h"
#include <Arduino.h>

MPU6886_AHRS::MPU6886_AHRS()
  : accelX_(0), accelY_(0), accelZ_(0),
    gyroX_(0), gyroY_(0), gyroZ_(0),
    temp_(0),
    gyroBiasX_(0), gyroBiasY_(0), gyroBiasZ_(0),
    roll_(0), pitch_(0), yaw_(0),
    lastUpdateMicros_(0) {
}

int MPU6886_AHRS::begin(TwoWire* wire, uint8_t address,
                         float sampleRateHz, float filterGain) {
  // センサーを初期化
  if (sensor_.begin(wire, address) != 0) {
    return -1;
  }

  // フィルタを初期化
  filter_.begin(sampleRateHz);
  filter_.setGain(filterGain);

  lastUpdateMicros_ = micros();
  return 0;
}

void MPU6886_AHRS::calibrateGyro(int samples) {
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < samples; i++) {
    float gx, gy, gz;
    sensor_.readGyro(&gx, &gy, &gz);
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(5);
  }

  gyroBiasX_ = sumX / samples;
  gyroBiasY_ = sumY / samples;
  gyroBiasZ_ = sumZ / samples;
}

void MPU6886_AHRS::update() {
  // センサーデータを読み取る
  sensor_.readAccel(&accelX_, &accelY_, &accelZ_);
  sensor_.readGyro(&gyroX_, &gyroY_, &gyroZ_);
  sensor_.readTemp(&temp_);

  // ジャイロバイアス補正を適用
  float correctedGx = gyroX_ - gyroBiasX_;
  float correctedGy = gyroY_ - gyroBiasY_;
  float correctedGz = gyroZ_ - gyroBiasZ_;

  // デルタ時間を計算
  uint32_t now = micros();
  float dtSec = (lastUpdateMicros_ == 0) ? 0.01f 
                : (now - lastUpdateMicros_) * 1.0e-6f;
  lastUpdateMicros_ = now;

  // フィルタを更新
  filter_.update(correctedGx, correctedGy, correctedGz,
                 accelX_, accelY_, accelZ_,
                 dtSec);

  // 姿勢を取得
  roll_ = filter_.getRoll();
  pitch_ = filter_.getPitch();
  yaw_ = filter_.getYaw();
}
