#pragma once
#include "App/App.h"
#include <M5CoreS3.h>
#include "UI/Switch/ToggleSwitch.h"
#include "UI/Button/Button.h"

class AppSetup : public App {
public:
    AppSetup();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void handlePress(int16_t x, int16_t y) override;
    void handleMove(int16_t x, int16_t y) override;
    void handleRelease(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const { return PURPLE; }
    uint16_t iconPressedColor() const { return DARKGREY; }
    uint16_t iconTextColor() const { return WHITE; }
    const char* appName() const override { return "Setup"; }

private:
    ToggleSwitch _serialModeToggle;  // Binary/Text切替
    ToggleSwitch _wifiToggle;        // WiFi ON/OFF
    ToggleSwitch _serialToggle;      // Serial ON/OFF
    ToggleSwitch _imuOutputToggle;   // IMU Output ON/OFF
    ButtonManager btnMgr;
    int _selectedBaudIndex = 3;  // デフォルト: 921600 (index 3)

    // UART受信履歴（最大32バイト）
    static constexpr int RX_HISTORY_SIZE = 32;
    uint8_t uart_rx_history[RX_HISTORY_SIZE] = {};
    int uart_rx_history_len = 0;

    // Ping応答状態
    bool ping_ok = false;
    unsigned long ping_ok_time = 0;

    void loadSettings();
    void saveSettings();
    void drawBaudSelector(M5Canvas &canvas, int x, int y);
    void handleBaudTouch(int16_t x, int16_t y);
};
