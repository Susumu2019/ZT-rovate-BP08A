/**
 * MPU6886_AHRS例 - 基本的な姿勢追跡
 * 
 * MPU6886_AHRSライブラリを使用したリアルタイム姿勢推定の
 * 基本的な使い方を示します。
 * 
 * ハードウェア設定:
 * - MPU6886をI2Cバスに接続
 * - SDAとSCLピンはWire.begin()で設定
 */

#include <Wire.h>
#include "MPU6886_AHRS.h"

MPU6886_AHRS imu;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("MPU6886 AHRS Example");
  Serial.println("====================");
  
  // I2Cバスを初期化（お使いのボードに合わせてピンを調整）
  // ESP32: Wire.begin(SDA, SCL)
  // Arduino: Wire.begin()
  Wire.begin(2, 1);  // 例: SDA=GPIO2, SCL=GPIO1
  
  // デフォルト設定でIMUを初期化
  // パラメータ: Wireオブジェクト、I2Cアドレス、サンプルレート、フィルタゲイン
  if (imu.begin(&Wire, 0x68, 100.0f, 0.4f) != 0) {
    Serial.println("ERROR: IMU initialization failed!");
    while (1) { delay(100); }
  }
  
  Serial.println("IMU initialized");
  
  // ジャイロスコープをキャリブレート（デバイスを静止状態に保ってください！）
  Serial.println("Calibrating gyroscope...");
  Serial.println("Keep device still for 1 second!");
  delay(500);
  
  imu.calibrateGyro(200);  // 200サンプルを平均化
  
  float bx, by, bz;
  imu.getGyroBias(&bx, &by, &bz);
  Serial.printf("Gyro bias: X=%.3f, Y=%.3f, Z=%.3f deg/s\n", bx, by, bz);
  Serial.println("Calibration complete!");
  Serial.println();
  
  delay(1000);
}

void loop() {
  // 姿勢を更新（自動的にdtを計算）
  imu.update();
  
  // 姿勢角を取得
  float roll = imu.getRoll();
  float pitch = imu.getPitch();
  float yaw = imu.getYaw();
  
  // 生センサーデータを取得
  float ax, ay, az;
  float gx, gy, gz;
  float temp;
  imu.getAccel(&ax, &ay, &az);
  imu.getGyro(&gx, &gy, &gz);
  imu.getTemp(&temp);
  
  // 姿勢を出力
  Serial.printf("Roll: %6.1f° | Pitch: %6.1f° | Yaw: %6.1f° | ",
                roll, pitch, yaw);
  
  // センサーデータを出力
  Serial.printf("Accel: %6.2f, %6.2f, %6.2f g | ",
                ax, ay, az);
  Serial.printf("Gyro: %6.1f, %6.1f, %6.1f dps | ",
                gx, gy, gz);
  Serial.printf("Temp: %4.1f°C\n", temp);
  
  delay(50);  // 20Hz更新レート（必要に応じて調整）
}
