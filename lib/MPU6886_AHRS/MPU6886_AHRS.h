/**
 * MPU6886 AHRS - 高レベル姿勢追跡クラス
 * MPU6886センサードライバとMadgwickフィルタを組み合わせ
 * 姿勢推定のための使いやすいAPIを提供
 */

#ifndef MPU6886_AHRS_H
#define MPU6886_AHRS_H

#include "MPU6886.h"
#include "MadgwickAHRS.h"

/**
 * オールインワンIMU姿勢トラッカー
 * センサー読み取り、キャリブレーション、姿勢推定を処理
 */
class MPU6886_AHRS {
public:
  MPU6886_AHRS();

  /**
   * 共有I2Cバスで初期化
   * @param wire I2Cバスへのポインタ (デフォルト: &Wire)
   * @param address I2Cアドレス (デフォルト: 0x68)
   * @param sampleRateHz フィルタ更新レート (デフォルト: 100)
   * @param filterGain Madgwickゲイン (デフォルト: 0.4)
   * @return 成功時0、失敗時-1
   */
  int begin(TwoWire* wire = &Wire,
            uint8_t address = 0x68,
            float sampleRateHz = 100.0f,
            float filterGain = 0.4f);

  /**
   * ジャイロスコープバイアスをキャリブレート（デバイスは静止状態を維持）
   * @param samples 平均化するサンプル数 (デフォルト: 200)
   */
  void calibrateGyro(int samples = 200);

  /**
   * 姿勢を更新（ループ内で呼び出す）
   * 前回の更新からのdtを自動的に計算
   */
  void update();

  /**
   * 姿勢角を取得（度数法）
   */
  float getRoll()  { return roll_; }
  float getPitch() { return pitch_; }
  float getYaw()   { return yaw_; }

  /**
   * 生センサーデータを取得（バイアス補正後）
   */
  void getAccel(float* ax, float* ay, float* az) {
    *ax = accelX_; *ay = accelY_; *az = accelZ_;
  }
  void getGyro(float* gx, float* gy, float* gz) {
    *gx = gyroX_ - gyroBiasX_;
    *gy = gyroY_ - gyroBiasY_;
    *gz = gyroZ_ - gyroBiasZ_;
  }
  void getTemp(float* temp) { *temp = temp_; }
  float getTemperature() { return temp_; }

  /**
   * ジャイロバイアス値を取得
   */
  void getGyroBias(float* bx, float* by, float* bz) {
    *bx = gyroBiasX_; *by = gyroBiasY_; *bz = gyroBiasZ_;
  }
  float getGyroBiasX() { return gyroBiasX_; }
  float getGyroBiasY() { return gyroBiasY_; }
  float getGyroBiasZ() { return gyroBiasZ_; }

  /**
   * ジャイロバイアスを手動で設定
   */
  void setGyroBias(float bx, float by, float bz) {
    gyroBiasX_ = bx; gyroBiasY_ = by; gyroBiasZ_ = bz;
  }

  /**
   * 姿勢を初期状態にリセット
   */
  void resetOrientation() { filter_.reset(); }

  /**
   * 内部センサーとフィルタへのアクセス
   */
  MPU6886& sensor() { return sensor_; }
  MadgwickAHRS& filter() { return filter_; }

private:
  MPU6886 sensor_;
  MadgwickAHRS filter_;

  float accelX_, accelY_, accelZ_;
  float gyroX_, gyroY_, gyroZ_;
  float temp_;

  float gyroBiasX_, gyroBiasY_, gyroBiasZ_;

  float roll_, pitch_, yaw_;

  uint32_t lastUpdateMicros_;
};

#endif // MPU6886_AHRS_H
