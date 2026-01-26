#pragma once
#include "App/App.h"
#include <M5CoreS3.h>
#include "UI/SliderBar/SliderBar.h"
#include "UI/Switch/ToggleSwitch.h"
#include "UI/Button/Button.h"

class AppMotor : public App {
public:
    AppMotor();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    // new pointer-based handlers
    void handlePress(int16_t x, int16_t y) override;
    void handleMove(int16_t x, int16_t y) override;
    void handleRelease(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const{ return DARKCYAN; };
    uint16_t iconPressedColor() const{ return DARKGREY; };
    uint16_t iconTextColor() const{ return WHITE; };
    const char* appName() const override { return "Motor"; }

    bool isSendingPulse() const { return _is_sending_pulse; }
    int getDirection() const { return _direction; }
    int getCurrentFrequency() const { return _current_frequency; }
private:
    SliderBar _freqSlider;
    ToggleSwitch _dir_toggle;
    ButtonManager btnMgr;
    int _current_frequency = 0;
    int _direction = 1; // 1: forward, -1: backward
    bool _is_sending_pulse = false;
};

class MotorControl {
public:
    bool isSendingPulse() const { return _is_sending_pulse; }
    int getDirection() const { return _direction; }
    int getCurrentFrequency() const { return _current_frequency; }
    void setSendingPulse(bool sending) { _is_sending_pulse = sending; }
    void setDirection(int dir) { _direction = dir; }
    void setCurrentFrequency(int freq) { _current_frequency = freq; }   
private:
    int _current_frequency = 0;
    int _direction = 1; // 1: forward, -1: backward
    bool _is_sending_pulse = false;
};

extern MotorControl motorControl;
    


