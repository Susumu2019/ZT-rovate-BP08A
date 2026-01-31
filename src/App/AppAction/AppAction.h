/**
 ****************************************************************************
 * @file     AppAction.h
 * @brief    AppAction class definition
 * @version  V1.0
 * @date     2026-01-12
 *****************************************************************************
 */
#pragma once
#include "App/App.h"
#include <M5CoreS3.h>
#include "UI/Button/Button.h"
#include <Adafruit_PWMServoDriver.h>

class AppAction : public App {
public:
    AppAction();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void handlePress(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const;
    uint16_t iconPressedColor() const;
    uint16_t iconTextColor() const;
    const char* appName() const override { return "Action"; }

private:
    void setServoAngle(int channel, int angle);
    void setModeData();
    void executeStep();
    void loadModeDataFromFile(const char* filename);
    void saveModeDataToFile(const char* filename);
    
    ButtonManager btnMgr;
    Adafruit_PWMServoDriver pwm;
    
    int selectedMode = 0; // 0:なし, 1:MODE_1, 2:MODE_2, 3:MODE_3
    bool isRunning = false;
    bool buttonsInitialized = false;  // ボタン初期化フラグ
    
    // モードデータ: [mode 0-2][servo 0-7][step 0-299]
    // 初期化リストで直接値を指定可能
    static const int MODE_COUNT = 3;
    static const int STEP_COUNT = 300;
    static const int SERVO_COUNT = 8;
    static int modeData[MODE_COUNT][SERVO_COUNT][STEP_COUNT];
    
    // サーボオフセット補正値（AppManualで設定したもの）
    int _servo_offsets[SERVO_COUNT] = {0};
    void loadOffsets();    // 補正値の読込（NVS）
    
    // 実行制御
    int currentStep = 0;
    unsigned long lastStepTime = 0;
    int STEP_INTERVAL_MS = 10;  // 定数から変数に変更（speedLevelに応じて動的に更新）
    
    // 速度制御（1-5段階: 1=最速100%, 5=最遅20%）
    int speedLevel = 1;  // デフォルト: 最速
    void updateStepInterval();  // speedLevelに応じてSTEP_INTERVAL_MSを更新
    int getStepIntervalMs() const;  // speedLevelに基づいた実際の待機時間を計算
    
    // サーボ設定
    static const int SERVO_MIN_PULSE = 375;
    static const int SERVO_MAX_PULSE = 2400;
};