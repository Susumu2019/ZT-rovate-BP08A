/**
 * Madgwick AHRSアルゴリズム
 * 6軸IMU（加速度計 + ジャイロ）用の姿勢フィルタ
 * プラットフォーム依存を持たないポータブル実装
 */

#ifndef MADGWICK_AHRS_H
#define MADGWICK_AHRS_H

#include <cmath>

/**
 * Madgwick姿勢フィルタ
 * 加速度計とジャイロスコープデータを融合して3D姿勢を推定
 */
class MadgwickAHRS {
public:
  MadgwickAHRS();

  /**
   * 予想サンプルレートでフィルタを初期化
   * @param sampleRateHz 更新周波数 (Hz)
   */
  void begin(float sampleRateHz);

  /**
   * 新しいIMU計測でフィルタを更新
   * @param gx, gy, gz ジャイロスコープ (deg/s)
   * @param ax, ay, az 加速度計 (g)
   * @param dtSec 前回の更新からの時間（秒）
   */
  void update(float gx, float gy, float gz,
              float ax, float ay, float az,
              float dtSec);

  /**
   * 設定されたサンプルレートを使用して更新（後方互換性）
   */
  void update(float gx, float gy, float gz,
              float ax, float ay, float az) {
    update(gx, gy, gz, ax, ay, az, invSampleFreq_);
  }

  /**
   * オイラー角として姿勢を取得（度数法）
   * Roll: X軸周りの回転
   * Pitch: Y軸周りの回転
   * Yaw: Z軸周りの回転
   */
  float getRoll();
  float getPitch();
  float getYaw();

  /**
   * クォータニオン要素を取得 (w, x, y, z)
   */
  void getQuaternion(float* q0, float* q1, float* q2, float* q3) {
    *q0 = q0_; *q1 = q1_; *q2 = q2_; *q3 = q3_;
  }

  /**
   * フィルタゲインを設定（収束速度）
   * 高い = 速い収束だがノイズが多い
   * 標準的: 0.1 - 0.5
   */
  void setGain(float gain) { beta_ = gain; }

  /**
   * 姿勢を初期状態にリセット
   */
  void reset() {
    q0_ = 1.0f;
    q1_ = 0.0f;
    q2_ = 0.0f;
    q3_ = 0.0f;
  }

private:
  float q0_, q1_, q2_, q3_;  // クォータニオン状態
  float beta_;                // フィルタゲイン
  float invSampleFreq_;       // 1 / サンプルレート（秒）

  static float invSqrt(float x);
};

#endif // MADGWICK_AHRS_H
