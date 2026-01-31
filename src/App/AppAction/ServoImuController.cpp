#include "ServoImuController.h"
#include <cmath>

ServoImuController::ServoImuController() {}

void ServoImuController::addData(const std::array<int, 8>& servoAngles, const ImuData& imu) {
    dataSet.push_back({servoAngles, imu});
}

void ServoImuController::clear() {
    dataSet.clear();
}

int ServoImuController::dataCount() const {
    return static_cast<int>(dataSet.size());
}

// 最も近いIMUデータのサーボ角度を返す（最近傍法）
std::array<int, 8> ServoImuController::predict(const ImuData& imu) const {
    if (dataSet.empty()) return {90,90,90,90,90,90,90,90};
    float minDist = 1e9;
    const DataPoint* best = nullptr;
    for (const auto& dp : dataSet) {
        float d = std::pow(dp.imu.ax - imu.ax, 2) + std::pow(dp.imu.ay - imu.ay, 2) + std::pow(dp.imu.az - imu.az, 2)
                + std::pow(dp.imu.gx - imu.gx, 2) + std::pow(dp.imu.gy - imu.gy, 2) + std::pow(dp.imu.gz - imu.gz, 2);
        if (d < minDist) {
            minDist = d;
            best = &dp;
        }
    }
    return best ? best->servoAngles : std::array<int,8>{90,90,90,90,90,90,90,90};
}
