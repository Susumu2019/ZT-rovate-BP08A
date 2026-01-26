#pragma once
#include "App/App.h"
#include <M5CoreS3.h>
//#include <Adafruit_PCA9685.h>
// #include <Adafruit_PWMServoDriver.h>
#include "UI/SliderBar/SliderBar.h"
#include "UI/Switch/ToggleSwitch.h"
#include "UI/Button/Button.h"

// サーボの定義（PCA9685 の 8 チャネル）
#define SERVO_COUNT 8
#define SERVO_MIN_PULSE 375   // 最小パルス幅（0度）
#define SERVO_MAX_PULSE 2400  // 最大パルス幅（180度）

// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


class AppManual : public App {
public:
    AppManual();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void handlePress(int16_t x, int16_t y) override;
    void handleMove(int16_t x, int16_t y) override;
    void handleRelease(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const{ return DARKCYAN; };
    uint16_t iconPressedColor() const{ return DARKGREY; };
    uint16_t iconTextColor() const{ return WHITE; };
    const char* appName() const override { return "Manual"; }

private:
    int _selected_servo = 0;  // 現在選択中のサーボ（0～7）
    uint16_t _servo_positions[SERVO_COUNT] = {0};  // 各サーボの現在位置（ユーザー指定角度）
    uint16_t _servo_offsets[SERVO_COUNT] = {0};    // 各サーボの中立補正値
    
    void setServoAngle(int channel, int angle);  // サーボを指定角度に設定
    void testAllServos();  // 全サーボの動作確認
    bool areAllServosAtNeutral() const;  // 全サーボが90度にあるか確認
    void loadOffsets();    // 補正値の読込（NVS）
    void saveOffsets();    // 補正値の保存（NVS）
};

