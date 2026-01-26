    // --- WiFi接続ボタン用 ---
    static constexpr int BTN_X = 80;
    static constexpr int BTN_Y = 200;
    static constexpr int BTN_W = 160;
    static constexpr int BTN_H = 40;
#pragma once
#include "App/App.h"
#include <M5CoreS3.h>

class AppWifi : public App {
public:
    AppWifi();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const;
    uint16_t iconPressedColor() const;
    uint16_t iconTextColor() const;
    const char* appName() const override { return "WiFi"; }
    const char* typeName() const override { return "AppWifi"; }

    void printNetworkInfo() const;
};
