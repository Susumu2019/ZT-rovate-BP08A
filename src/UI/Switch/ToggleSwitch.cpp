#include "ToggleSwitch.h"
#include <cstdio>
#include "system/touch/TouchManager.h"

ToggleSwitch::ToggleSwitch(int x, int y, int w, int h, bool initial,
                                                     uint16_t bgOff, uint16_t bgOn,
                                                     uint16_t knobColor, uint16_t textColor,
                                                     const char* labelOn, const char* labelOff)
        : _x(x), _y(y), _w(w), _h(h), _value(initial), _is_pressed(false),
            _bgOff(bgOff), _bgOn(bgOn), _knobColor(knobColor), _textColor(textColor), _cb(nullptr),
            _labelOn(labelOn), _labelOff(labelOff) {}

void ToggleSwitch::draw(M5Canvas &canvas) {
    // background rounded rect
    // determine current position (0.0 off .. 1.0 on)
    float pos = _value ? 1.0f : 0.0f;
    if (_animating) {
        uint32_t now = millis();
        uint32_t dt = now - _animStartMs;
        if (dt >= _animDurationMs) {
            // finish
            _animating = false;
            _value = _targetValue;
            pos = _value ? 1.0f : 0.0f;
            if (_cb) _cb(_value);
        } else {
            float t = (float)dt / (float)_animDurationMs;
            // simple ease-out (smooth)
            float ease = 1 - (1 - t) * (1 - t);
            pos = _animStartPos + (_animTargetPos - _animStartPos) * ease;
        }
    }

    uint16_t bg = (pos > 0.5f) ? _bgOn : _bgOff;
    int radius = _h / 2;
    canvas.fillRoundRect(_x, _y, _w, _h, radius, bg);

    // knob position: left when off, right when on
    int knobR = _h / 2 ; // knob radius
    // compute knob X by interpolating between left and right positions
    int leftX = _x + knobR + 2;
    int rightX = _x + _w - knobR - 2;
    int knobX = (int)(leftX + (rightX - leftX) * pos + 0.5f);
    int knobY = _y + _h / 2;

    // draw knob with slight shadow
    canvas.fillCircle(knobX, knobY, knobR + 1, 0x4208); // shadow/darken
    canvas.fillCircle(knobX, knobY, knobR, _knobColor);

    // optional text (small 'ON'/'OFF') centered vertically, near center
    canvas.setTextSize(1);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(_textColor);
    const char* label = _value ? _labelOn.c_str() : _labelOff.c_str();
    canvas.drawCentreString(label, _x + _w / 2, _y + _h / 2 - 6);
}

void ToggleSwitch::update() {
    const auto &t = touchManager.getTouchInfo();
    // On initial press, claim the press if it started inside this widget.
    if (t.wasPressed) {
        if (isTouched(t.x, t.y)) {
            _is_pressed = true;
        } else {
            _is_pressed = false;
        }
        return;
    }

    // While pressed, do nothing here; we keep ownership in _is_pressed.
    if (t.isPressed) {
        return;
    }

    // On release: if this widget owned the press and the release occurred
    // inside the widget, start the toggle animation (i.e. toggle on release).
    if (t.wasReleased) {
        if (_is_pressed && isTouched(t.x, t.y)) {
            _targetValue = !_value;
            _animating = true;
            _animStartMs = millis();
            _animStartPos = _value ? 1.0f : 0.0f;
            _animTargetPos = _targetValue ? 1.0f : 0.0f;
            if (_animDurationMs == 0) {
                _animating = false;
                _value = _targetValue;
                if (_cb) _cb(_value);
            }
        }
        _is_pressed = false;
        return;
    }
}

bool ToggleSwitch::isTouched(int touchX, int touchY) const {
    return (touchX >= _x && touchX <= _x + _w && touchY >= _y && touchY <= _y + _h);
}

void ToggleSwitch::setValue(bool v) {
    if (_value != v) {
        _value = v;
        if (_cb) _cb(_value);
    }
}
