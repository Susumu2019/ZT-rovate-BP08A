/**
 ****************************************************************************
 * @file     AppTemplate.h
 * @brief    AppTemplate class definition
 * @note     アプリのヘッダーファイルのテンプレートです。新しいアプリを作成する際の基礎として使用してください。
 * @version  V1.0
 * @date     2025-10-3
 * @author   Susumu Hirai
 *****************************************************************************
 */
#pragma once
#include "App/App.h"
#include <M5CoreS3.h>

class AppTemplate : public App {
public:
    AppTemplate();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const;
    uint16_t iconPressedColor() const;
    uint16_t iconTextColor() const;
    const char* appName() const override { return "Template"; }
};
