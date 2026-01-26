#pragma once
#include <M5CoreS3.h>
#include <functional>
#include "color_config.h"

class ToggleSwitch {
public:
    using Callback = std::function<void(bool)>;

    ToggleSwitch(int x, int y, int w, int h, bool initial = false,
                 uint16_t bgOff = 0x8410, uint16_t bgOn = DARKCYAN,
                 uint16_t knobColor = WHITE, uint16_t textColor = WHITE,
                 const char* labelOn = "ON", const char* labelOff = "OFF");

    void draw(M5Canvas &canvas);
    void update();
    bool isTouched(int touchX, int touchY) const;

    void setValue(bool v);
    bool getValue() const { return _value; }

    void setCallback(Callback cb) { _cb = std::move(cb); }

private:
    int _x, _y, _w, _h;
    bool _value;
    bool _is_pressed;
    bool _was_pressed;
    bool _was_released;
    int _touch_X, _touch_Y;


    uint16_t _bgOff;
    uint16_t _bgOn;
    uint16_t _knobColor;
    uint16_t _textColor;
    Callback _cb;

    // animation
    bool _animating = false;
    bool _targetValue = false;
    uint32_t _animStartMs = 0;
    uint32_t _animDurationMs = 200; // ms
    float _animStartPos = 0.0f; // 0.0 (off) .. 1.0 (on)
    float _animTargetPos = 0.0f;
    // labels for on/off state
    String _labelOn = "ON";
    String _labelOff = "OFF";
};
