#include "Settings.h"

void Settings::begin() {
    prefs_.begin("rovate", false);  // 名前空間 "rovate"
    
    // 設定読み込み（デフォルト値あり）
    serialMode_ = (SerialMode)prefs_.getUChar("serialMode", SERIAL_BINARY);
    wifiEnabled_ = prefs_.getBool("wifiEnabled", true);
    serialEnabled_ = prefs_.getBool("serialEnabled", true);
    imuOutputEnabled_ = prefs_.getBool("imuOutput", true);
    controlRate_ = prefs_.getUShort("controlRate", 100);
    serialBaud_ = prefs_.getULong("serialBaud", 921600);
    
    Serial.println("Settings: loaded from NVS");
    Serial.printf("  Serial Mode: %s\n", serialMode_ == SERIAL_BINARY ? "Binary" : "Text");
    Serial.printf("  WiFi: %s\n", wifiEnabled_ ? "ON" : "OFF");
    Serial.printf("  Serial: %s\n", serialEnabled_ ? "ON" : "OFF");
    Serial.printf("  IMU Output: %s\n", imuOutputEnabled_ ? "ON" : "OFF");
    Serial.printf("  Control Rate: %d Hz\n", controlRate_);
    Serial.printf("  Serial Baud: %lu bps\n", (unsigned long)serialBaud_);
}

void Settings::save() {
    prefs_.putUChar("serialMode", (uint8_t)serialMode_);
    prefs_.putBool("wifiEnabled", wifiEnabled_);
    prefs_.putBool("serialEnabled", serialEnabled_);
    prefs_.putBool("imuOutput", imuOutputEnabled_);
    prefs_.putUShort("controlRate", controlRate_);
    prefs_.putULong("serialBaud", serialBaud_);
    
    Serial.println("Settings: saved to NVS");
}
