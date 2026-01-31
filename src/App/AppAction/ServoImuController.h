#pragma once
#include <vector>
#include <array>

struct ImuData {
    float ax, ay, az;
    float gx, gy, gz;
    float temp;
};

class ServoImuController {
public:
    // サーボ角度とIMUデータのセット
    struct DataPoint {
        std::array<int, 8> servoAngles;
        ImuData imu;
    };

    ServoImuController();
    void addData(const std::array<int, 8>& servoAngles, const ImuData& imu);
    std::array<int, 8> predict(const ImuData& imu) const;
    void clear();
    int dataCount() const;

private:
    std::vector<DataPoint> dataSet;
};
