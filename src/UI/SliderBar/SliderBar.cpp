#include "SliderBar.h"
#include <cstdio>
#include "color_config.h"
#include "system/touch/TouchManager.h"

SliderBar::SliderBar(int x, int y, int w, int h, int initialValue, int maxValue, int minValue,
                                         uint16_t trackColor, uint16_t fillColor,
                                         uint16_t knobColor, uint16_t knobPressedColor,
                                         uint16_t textColor)
        : _x(x), _y(y), _w(w), _h(h), _value(initialValue), _maxValue(maxValue), _minValue(minValue), _pressed(false),
            _trackColor(trackColor), _fillColor(fillColor), _knobColor(knobColor),
            _knobPressedColor(knobPressedColor), _textColor(textColor) {
    // Ensure min <= max; swap if necessary
    if (_minValue > _maxValue) {
        int tmp = _minValue; _minValue = _maxValue; _maxValue = tmp;
    }
    // Clamp initial value into range
    if (_value < _minValue) _value = _minValue;
    if (_value > _maxValue) _value = _maxValue;
}

void SliderBar::draw(M5Canvas &canvas) {
    // トラック
    canvas.fillRoundRect(_x, _y, _w, _h, _h / 2, _trackColor);
    // 充填部分 — レンジに基づく幅計算
    int rangeSpan = _maxValue - _minValue;
    int filledW = 0;
    if (rangeSpan > 0) {
        filledW = ((_value - _minValue) * _w) / rangeSpan;
    } else {
        // fallback to percent-style
        filledW = (_value * _w) / 100;
    }
    if (filledW > 0) {
        canvas.fillRoundRect(_x, _y, filledW, _h, _h / 2, _fillColor);
    }
    // ノブ
    int knobX = _x + filledW;
    int knobR = (int)(_h * 1.2f);
    if (knobX < _x) knobX = _x;
    if (knobX > _x + _w) knobX = _x + _w;
    canvas.fillCircle(knobX, _y + _h / 2, knobR, _pressed ? _knobPressedColor : _knobColor);
    // 値表示 — レンジに対する実値を表示
    char valStr[16];
    std::sprintf(valStr, "%d", _value);
    canvas.setTextSize(1);
    canvas.setTextColor(_textColor);
    canvas.drawCentreString(valStr, _x + _w / 2, _y - 18);
}

bool SliderBar::isTouched(int touchX, int touchY) const {
    int knobR = _h * 2;
    int left = _x - knobR;
    int right = _x + _w + knobR;
    int top = _y - knobR - 6;
    int bottom = _y + _h + knobR;
    return (touchX >= left && touchX <= right && touchY >= top && touchY <= bottom);
}

bool SliderBar::setValueFromTouch(int touchX) {
    int rel = touchX - _x;
    if (rel < 0) rel = 0;
    if (rel > _w) rel = _w;
    int rangeSpan = _maxValue - _minValue;
    int newVal;
    if (rangeSpan > 0) {
        newVal = _minValue + (rel * rangeSpan) / _w;
    } else {
        newVal = (rel * 100) / _w;
    }
    if (newVal < _minValue) newVal = _minValue;
    if (newVal > _maxValue) newVal = _maxValue;
    if (newVal != _value) {
        _value = newVal;
        if (_onChange) _onChange(_value);
        return true;
    }
    return false;
}

int SliderBar::getValue() const { return _value; }

void SliderBar::setOnChange(Callback cb) {
    _onChange = std::move(cb);
}

void SliderBar::update() {
    const auto &t = touchManager.getTouchInfo();
    if (t.wasPressed) {
        // If the initial press is within this slider, enter pressed state
        if (isTouched(t.x, t.y)) {
            _pressed = true;
            setValueFromTouch(t.x);
        } else {
            _pressed = false;
        }
    } else if (t.isPressed) {
        // While pressed, only update if this slider currently owns the press
        if (_pressed) {
            setValueFromTouch(t.x);
        }
    } else if (t.wasReleased) {
        // Clear pressed flag on release
        _pressed = false;
    }
}

void SliderBar::setValue(int v) {
    int newV = v;
    if (newV < _minValue) newV = _minValue;
    if (newV > _maxValue) newV = _maxValue;
    if (newV != _value) {
        _value = newV;
        if (_onChange) _onChange(_value);
    }
}

void SliderBar::setRange(int minValue, int maxValue) {
    if (minValue > maxValue) {
        int tmp = minValue; minValue = maxValue; maxValue = tmp;
    }
    _minValue = minValue;
    _maxValue = maxValue;
    if (_value < _minValue) _value = _minValue;
    if (_value > _maxValue) _value = _maxValue;
}

