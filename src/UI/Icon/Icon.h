#pragma once
#include <M5CoreS3.h>

class Icon {
public:
    Icon();
    void drawArrow(int x, int y, int length, int color, const char* direction);
    void drawCommonIcon(const char *label, int x, int y, int w, int h, int r, int color);
};

extern Icon icon;

