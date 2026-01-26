#pragma once

#include <M5CoreS3.h>
#include <functional>
#include "color_config.h"

class SliderBar  {
public:
    // Colors are 16-bit (RGB565). Defaults keep previous look.
    SliderBar(int x, int y, int w, int h, int initialValue = 50, int maxValue = 100, int minValue = 0,
              uint16_t trackColor = 0x7BEF,
              uint16_t fillColor = DARKCYAN,
              uint16_t knobColor = DARKCYAN,
              uint16_t knobPressedColor = DARKER_CYAN,
              uint16_t textColor = WHITE);

    void draw(M5Canvas &canvas);
    using Callback = std::function<void(int)>;

    bool isTouched(int touchX, int touchY) const;
    bool setValueFromTouch(int touchX);
    void setValue(int v);
    void setRange(int minValue, int maxValue);
    void update();
    int getValue() const;
    void setOnChange(Callback cb);

private:
    int _x, _y, _w, _h;
    int _value;
    int _maxValue;
    int _minValue;
    bool _pressed;
    // customizable colors
    uint16_t _trackColor;
    uint16_t _fillColor;
    uint16_t _knobColor;
    uint16_t _knobPressedColor;
    uint16_t _textColor;
    // change callback
    Callback _onChange;
};