/**
 * MPU6886 IMUセンサードライバ
 * MPU6886 6軸IMU用のポータブルライブラリ
 * Arduino Wireライブラリを持つ全てのマイコンと互換性あり
 */

#ifndef MPU6886_H
#define MPU6886_H

#include <Arduino.h>
#include <Wire.h>

// MPU6886 レジスタマップ
#define MPU6886_ADDRESS           0x68
#define MPU6886_WHOAMI            0x75
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_XOUT_H      0x3B
#define MPU6886_TEMP_OUT_H        0x41
#define MPU6886_GYRO_XOUT_H       0x43
#define MPU6886_USER_CTRL         0x6A
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_FIFO_EN           0x23
#define MPU6886_FIFO_COUNT        0x72
#define MPU6886_FIFO_R_W          0x74
#define MPU6886_GYRO_OFFSET       0x13

/**
 * MPU6886 6軸IMUドライバクラス
 * 加速度計、ジャイロスコープ、温度センサーをサポート
 */
class MPU6886 {
public:
  enum AccelScale { AFS_2G = 0, AFS_4G, AFS_8G, AFS_16G };
  enum GyroScale { GFS_250DPS = 0, GFS_500DPS, GFS_1000DPS, GFS_2000DPS };

  MPU6886();

  /**
   * 共有I2CバスでMPU6886を初期化
   * @param wire TwoWireオブジェクトへのポインタ (Wire, Wire1, など)
   * @param address I2Cデバイスアドレス (デフォルト: 0x68)
   * @return 成功時0、失敗時-1
   */
  int begin(TwoWire* wire = &Wire, uint8_t address = MPU6886_ADDRESS);

  /**
   * 生ADC値を読み取る
   */
  void readAccelADC(int16_t* ax, int16_t* ay, int16_t* az);
  void readGyroADC(int16_t* gx, int16_t* gy, int16_t* gz);
  void readTempADC(int16_t* temp);

  /**
   * キャリブレーション済みセンサーデータを読み取る
   * 加速度: g (9.8m/s²)、ジャイロ: deg/s、温度: °C
   */
  void readAccel(float* ax, float* ay, float* az);
  void readGyro(float* gx, float* gy, float* gz);
  void readTemp(float* temp);

  /**
   * センサー範囲を設定
   */
  void setGyroScale(GyroScale scale);
  void setAccelScale(AccelScale scale);

  /**
   * FIFO操作
   */
  void setFIFOEnabled(bool enable);
  uint16_t getFIFOCount();
  uint8_t readFIFO();
  void readFIFOBuffer(uint8_t* buffer, uint16_t length);
  void resetFIFO();

  /**
   * ジャイロオフセットキャリブレーション
   */
  void setGyroOffset(uint16_t x, uint16_t y, uint16_t z);

  /**
   * チップIDを取得
   */
  uint8_t getDeviceID() { return deviceID_; }

  /**
   * 現在のスケール解像度を取得
   */
  float getAccelRes() { return accelRes_; }
  float getGyroRes() { return gyroRes_; }

private:
  TwoWire* wire_;
  uint8_t i2cAddress_;
  uint8_t deviceID_;
  
  AccelScale accelScale_;
  GyroScale gyroScale_;
  float accelRes_;
  float gyroRes_;

  void readBytes(uint8_t reg, uint8_t count, uint8_t* buffer);
  void writeBytes(uint8_t reg, uint8_t count, uint8_t* buffer);
  void writeByte(uint8_t reg, uint8_t value);
  void updateAccelRes();
  void updateGyroRes();
};

#endif // MPU6886_H
