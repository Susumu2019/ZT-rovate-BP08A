/**
 * Madgwick AHRSアルゴリズム実装
 */

#include "MadgwickAHRS.h"

MadgwickAHRS::MadgwickAHRS()
  : q0_(1.0f), q1_(0.0f), q2_(0.0f), q3_(0.0f),
    beta_(0.1f), invSampleFreq_(0.01f) {
}

void MadgwickAHRS::begin(float sampleRateHz) {
  invSampleFreq_ = 1.0f / sampleRateHz;
}

float MadgwickAHRS::invSqrt(float x) {
  // 高速逆平方根（Quake IIIアルゴリズム）
  float halfx = 0.5f * x;
  float y = x;
  long i = *(long*)&y;
  i = 0x5f3759df - (i >> 1);
  y = *(float*)&i;
  y = y * (1.5f - (halfx * y * y));
  return y;
}

void MadgwickAHRS::update(float gx, float gy, float gz,
                          float ax, float ay, float az,
                          float dtSec) {
  // ジャイロをdeg/sからrad/sに変換
  gx *= 0.01745329f;
  gy *= 0.01745329f;
  gz *= 0.01745329f;

  // 加速度計計測を正規化
  float norm = invSqrt(ax * ax + ay * ay + az * az);
  ax *= norm;
  ay *= norm;
  az *= norm;

  // 推定重力方向（半分）
  float halfvx = q1_ * q3_ - q0_ * q2_;
  float halfvy = q0_ * q1_ + q2_ * q3_;
  float halfvz = q0_ * q0_ - 0.5f + q3_ * q3_;

  // 誤差は推定重力と計測重力の外積
  float halfex = (ay * halfvz - az * halfvy);
  float halfey = (az * halfvx - ax * halfvz);
  float halfez = (ax * halfvy - ay * halfvx);

  // フィードバックゲインを適用
  halfex *= beta_;
  halfey *= beta_;
  halfez *= beta_;

  // フィードバックをジャイロレートに統合
  gx += 2.0f * halfex;
  gy += 2.0f * halfey;
  gz += 2.0f * halfez;

  // クォータニオンレートを統合
  float halfdt = 0.5f * dtSec;

  float qa = q0_ + (-q1_ * gx - q2_ * gy - q3_ * gz) * halfdt;
  float qb = q1_ + ( q0_ * gx + q3_ * gy - q2_ * gz) * halfdt;
  float qc = q2_ + (-q3_ * gx + q0_ * gy + q1_ * gz) * halfdt;
  float qd = q3_ + ( q2_ * gx - q1_ * gy + q0_ * gz) * halfdt;

  // クォータニオンを正規化
  norm = invSqrt(qa * qa + qb * qb + qc * qc + qd * qd);
  q0_ = qa * norm;
  q1_ = qb * norm;
  q2_ = qc * norm;
  q3_ = qd * norm;
}

float MadgwickAHRS::getRoll() {
  float roll = atan2f(2.0f * (q0_ * q1_ + q2_ * q3_),
                      1.0f - 2.0f * (q1_ * q1_ + q2_ * q2_));
  return roll * 57.29578f;  // radをdegに変換
}

float MadgwickAHRS::getPitch() {
  float pitch = asinf(2.0f * (q0_ * q2_ - q3_ * q1_));
  return pitch * 57.29578f;  // radをdegに変換
}

float MadgwickAHRS::getYaw() {
  float yaw = atan2f(2.0f * (q0_ * q3_ + q1_ * q2_),
                     1.0f - 2.0f * (q2_ * q2_ + q3_ * q3_));
  return yaw * 57.29578f;  // radをdegに変換
}
