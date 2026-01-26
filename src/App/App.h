#pragma once
#include <M5CoreS3.h>

// Fallback color definitions if not provided by platform headers
#ifndef DARKBLUE
#define DARKBLUE 0x0011
#endif
#ifndef DARKGREEN
#define DARKGREEN 0x03E0
#endif

class App {
public:
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void draw(M5Canvas& canvas) = 0;
    // New unified touch handler (coordinates)
    virtual void handleTouch(int16_t x, int16_t y) { onTouch((int)x, (int)y); }
    // Deprecated: old signature kept for compatibility
    virtual void onTouch(int x, int y) {}
    // draw app icon for home screen at position x,y with size w,h
    // 'pressed' is true while the icon is being pressed
    virtual void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
        // default: simple rounded rect with app name
        uint16_t bg = pressed ? DARKGREY : DARKGREY;
        canvas.fillRoundRect(x, y, w, h, 8, bg);
        canvas.setTextColor(WHITE, bg);
        canvas.setTextSize(1);
        canvas.setFont(&fonts::Font2);
        canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
    }
    // Per-app colors for home icon buttons. Override in specific apps if desired.
    virtual uint16_t iconBackgroundColor() const { return DARKGREY; }
    virtual uint16_t iconPressedColor() const { return DARKGREY; }
    virtual uint16_t iconTextColor() const { return WHITE; }
    // app display name
    virtual const char* appName() const { return "App"; }
    virtual ~App() {}
    // RTTIが無効な環境でも型判定するための識別子
    virtual const char* typeName() const { return "App"; }
    // touch press/move/release handlers specific to pointer interaction
    virtual void handlePress(int16_t x, int16_t y) {}
    virtual void handleMove(int16_t x, int16_t y) {}
    virtual void handleRelease(int16_t x, int16_t y) { onTouch((int)x, (int)y); }
};
