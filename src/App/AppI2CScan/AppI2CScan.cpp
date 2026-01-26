/**
 ****************************************************************************
 * @file     AppI2CScan.cpp
 * @brief    I2C Scanner - I2Cデバイススキャンアプリ実装
 * @version  V1.0
 * @date     2026-01-21
 * @author   System
 *****************************************************************************
 */

#include "AppI2CScan.h"
#include "config.h"
#include "../../system/system.h"

AppI2CScan::AppI2CScan() : device_count_(0) {}

void AppI2CScan::setup() {
    mode = 300;
    scanning_ = true;
    scan_start_time_ = millis();
    device_count_ = 0;
    current_port_ = 0;
    scroll_offset_ = 0;
}

void AppI2CScan::loop() {
    // I2Cスキャンを実行中
    if (scanning_) {
        // Wire（PORT.A 外部I2C）をスキャン
        if (current_port_ == 0) {
            scanI2CPort(Wire, "Wire (PORT.A)", SDA_PIN, SCL_PIN);
            current_port_++;
        }
        // Wire1（内部I2C）をスキャン
        else if (current_port_ == 1) {
            scanI2CPort(Wire1, "Wire1 (internal)", 21, 22);
            current_port_++;
            scanning_ = false;  // スキャン完了
        }
    }
}

const char* AppI2CScan::identifyDevice(uint8_t address) {
    // M5CoreS3に実装されているI2Cデバイスと汎用デバイスを認識
    switch (address) {
        // M5CoreS3内部実装デバイス
        case 0x34: return "AXP2101 (Power Mgmt)";
        case 0x36: return "AW88298 (Audio Amp)";
        case 0x38: return "FT6336U (Touch)";
        case 0x51: return "PCF8563 (RTC)";
        case 0x58: return "AW9523B/ILI9342C";
        case 0x5D: return "GT911 (Touch)";
        case 0x68: return "MPU6886 (IMU)";
        
        // 汎用デバイス
        case 0x69: return "MPU6886 alt";
        case 0x48: return "ADS1115";
        case 0x54: return "STAMPS3 (R)";
        case 0x3C: return "SSD1306";
        case 0x27: return "PCF8574";
        case 0x20: return "MCP23017";
        case 0x76: return "BME280";
        case 0x40: return "PCA9685";
        default: return "Unknown";
    }
}

void AppI2CScan::scanI2CPort(TwoWire& wire, const char* port_name, uint8_t sda_pin, uint8_t scl_pin) {
    int found = 0;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        wire.beginTransmission(addr);
        uint8_t error = wire.endTransmission();
        
        if (error == 0) {
            if (device_count_ < MAX_DEVICES) {
                found_devices_[device_count_].address = addr;
                found_devices_[device_count_].name = identifyDevice(addr);
                found_devices_[device_count_].port_name = port_name;
                found_devices_[device_count_].sda_pin = sda_pin;
                found_devices_[device_count_].scl_pin = scl_pin;
                device_count_++;
                found++;
            }
        }
    }
}

void AppI2CScan::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    canvas.setFont(&fonts::Font2);
    canvas.setTextSize(1);

    if (scanning_) {
        canvas.setTextColor(CYAN);
        canvas.drawString("=== I2C Scanning ===", 10, 30);

        canvas.setTextColor(YELLOW);
        canvas.drawString("Please wait...", 10, 70);

        canvas.setTextColor(WHITE);
        if (current_port_ == 0) {
            canvas.drawString("Scanning: Wire (PORT.A)", 10, 120);
            canvas.drawString("SDA=GPIO2, SCL=GPIO1", 10, 140);
        } else if (current_port_ == 1) {
            canvas.drawString("Scanning: Wire1 (internal)", 10, 120);
            canvas.drawString("SDA=GPIO21, SCL=GPIO22", 10, 140);
        }
        return;
    }

    // スキャン完了時の表示
    int y = 8;
    char buf[64];

    canvas.setTextColor(CYAN);
    canvas.drawString("I2C Scanner", 10, y);

    canvas.setTextColor(GREEN);
    sprintf(buf, "Devices: %d", device_count_);
    canvas.drawString(buf, 180, y);

    y = 32;

    if (device_count_ == 0) {
        canvas.setTextColor(RED);
        canvas.drawString("No devices found", 10, 120);
        canvas.setTextColor(DARKGREY);
        canvas.drawString("Tap to rescan", 10, LCD_HEIGHT - 24);
        return;
    }

    const int max_display = MAX_DISPLAY;  // 4件/ページ（2行構成で収める）
    int displayed = 0;

    for (int i = scroll_offset_; i < device_count_ && displayed < max_display; i++) {
        // 1行目: アドレス＋ポート
        canvas.setTextColor(CYAN);
        sprintf(buf, "0x%02X [%s]", found_devices_[i].address, found_devices_[i].port_name);
        canvas.drawString(buf, 10, y);

        // 2行目: デバイス名＋ピン番号
        canvas.setTextColor(WHITE);
        sprintf(buf, "%s  (SDA=%d, SCL=%d)",
                found_devices_[i].name,
                found_devices_[i].sda_pin,
                found_devices_[i].scl_pin);
        canvas.drawString(buf, 10, y + 16);

        y += 38;  // 2行で38pxの高さに統一
        displayed++;
    }

    // スクロール情報と操作ガイド
    canvas.setTextColor(DARKGREY);
    int total_pages = (device_count_ + max_display - 1) / max_display;
    int page = (scroll_offset_ / max_display) + 1;
    sprintf(buf, "Page %d/%d", page, total_pages);
    canvas.drawString(buf, 10, LCD_HEIGHT - 24);

    if (total_pages > 1) {
        canvas.drawString("Left:Prev  Right:Next", 110, LCD_HEIGHT - 24);
    } else {
        canvas.drawString("Tap to rescan", 110, LCD_HEIGHT - 24);
    }
}

void AppI2CScan::onTouch(int x, int y) {
    if (scanning_) return;

    const int max_display = MAX_DISPLAY;
    int total_pages = (device_count_ + max_display - 1) / max_display;

    // 下部タップでページ送り（左:前ページ / 右:次ページ）
    if (device_count_ > max_display && y > LCD_HEIGHT - 40) {
        int page = (scroll_offset_ / max_display);
        if (x < LCD_WIDTH / 2) {
            // 前ページ
            if (page > 0) {
                scroll_offset_ = (page - 1) * max_display;
            }
        } else {
            // 次ページ
            if (page + 1 < total_pages) {
                scroll_offset_ = (page + 1) * max_display;
            }
        }
        return;
    }

    // それ以外のタップはリスキャン
    device_count_ = 0;
    scroll_offset_ = 0;
    current_port_ = 0;
    scanning_ = true;
    scan_start_time_ = millis();
}

void AppI2CScan::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppI2CScan::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    uint16_t bg = pressed ? DARKBLUE : BLUE;
    canvas.fillRoundRect(x, y, w, h, 10, bg);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
}

uint16_t AppI2CScan::iconBackgroundColor() const { return DARKBLUE; }
uint16_t AppI2CScan::iconPressedColor() const { return DARKGREY; }
uint16_t AppI2CScan::iconTextColor() const { return WHITE; }

/*********************************** END OF FILE ******************************/
