/**
 ****************************************************************************
 * @file     AppI2CScan.h
 * @brief    I2C Scanner - I2Cデバイススキャンアプリ
 * @version  V1.0
 * @date     2026-01-21
 * @author   System
 *****************************************************************************
 */

#ifndef APP_I2CSCAN_H
#define APP_I2CSCAN_H

#include <M5CoreS3.h>
#include "../App.h"
#include <Wire.h>

class AppI2CScan : public App {
public:
    AppI2CScan();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const;
    uint16_t iconPressedColor() const;
    uint16_t iconTextColor() const;
    const char* appName() const override { return "I2C Scan"; }
    const char* typeName() const override { return "I2CScan"; }

private:
    static const int MAX_DEVICES = 32;
    static const int MAX_DISPLAY = 4;  // 表示・ページングに使う1ページあたり件数
    
    struct I2CDevice {
        uint8_t address;
        const char* name;
        const char* port_name;
        uint8_t sda_pin;
        uint8_t scl_pin;
    };
    
    I2CDevice found_devices_[MAX_DEVICES];
    int device_count_ = 0;
    
    // スキャン状態
    bool scanning_ = false;
    unsigned long scan_start_time_ = 0;
    int current_port_ = 0;  // 0 = Wire(PORT.A), 1 = Wire1(internal)
    
    // 結果表示用
    int scroll_offset_ = 0;
    
    void scanI2CPort(TwoWire& wire, const char* port_name, uint8_t sda_pin, uint8_t scl_pin);
    const char* identifyDevice(uint8_t address);
};

#endif // APP_I2CSCAN_H

/*********************************** END OF FILE ******************************/
