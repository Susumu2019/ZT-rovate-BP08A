#pragma once
#include <Preferences.h>
#include <Arduino.h>

/**
 * @brief 設定管理クラス（NVS使用、電源OFF後も保持）
 */
class Settings {
public:
    enum SerialMode {
        SERIAL_BINARY = 0,  // バイナリ通信
        SERIAL_TEXT = 1     // テキスト（JSON）通信
    };

    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    void begin();
    void save();

    // シリアル通信モード
    SerialMode getSerialMode() const { return serialMode_; }
    void setSerialMode(SerialMode mode) { serialMode_ = mode; }

    // WiFi有効/無効
    bool isWifiEnabled() const { return wifiEnabled_; }
    void setWifiEnabled(bool enabled) { wifiEnabled_ = enabled; }

    // シリアル有効/無効
    bool isSerialEnabled() const { return serialEnabled_; }
    void setSerialEnabled(bool enabled) { serialEnabled_ = enabled; }

    // 制御レート
    uint16_t getControlRate() const { return controlRate_; }
    void setControlRate(uint16_t rate) { controlRate_ = rate; }

    // シリアルボーレート
    uint32_t getSerialBaud() const { return serialBaud_; }
    void setSerialBaud(uint32_t baud) { serialBaud_ = baud; }

    // IMU出力有効/無効
    bool isImuOutputEnabled() const { return imuOutputEnabled_; }
    void setImuOutputEnabled(bool enabled) { imuOutputEnabled_ = enabled; }

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    Preferences prefs_;
    SerialMode serialMode_ = SERIAL_BINARY;
    bool wifiEnabled_ = true;
    bool serialEnabled_ = true;
    bool imuOutputEnabled_ = true;  // IMUデータ出力
    uint16_t controlRate_ = 100;  // Hz
    uint32_t serialBaud_ = 921600;  // bps
};
