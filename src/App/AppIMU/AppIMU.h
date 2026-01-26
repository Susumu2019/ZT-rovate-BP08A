#ifndef APP_IMU_H
#define APP_IMU_H

#include <M5CoreS3.h>
#include "../App.h"
#include "MPU6886_AHRS.h"

// IMU (filtered) defined in main.cpp
extern MPU6886_AHRS imu6886_ahrs;


class AppIMU : public App {
public:
    enum UpAxis { UP_X = 0, UP_Y = 1, UP_Z = 2 };
    AppIMU();
    void setup() override;
    void loop() override;
    void draw(M5Canvas &canvas) override;
    void onTouch(int x, int y) override;
    void handleTouch(int16_t x, int16_t y) override;
    void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override;
    uint16_t iconBackgroundColor() const;
    uint16_t iconPressedColor() const;
    uint16_t iconTextColor() const;
    const char* appName() const override { return "IMU"; }
    const char* typeName() const override { return "IMU"; }

private:
    UpAxis upAxis = UP_Z;
    int buttonHitTest(int x, int y) const;
};

#endif
