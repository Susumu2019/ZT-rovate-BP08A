#include "Icon.h"

extern M5Canvas canvas;
Icon icon;
Icon::Icon() {}

/**
 * 汎用矢印描画関数（向きをテキストで指定）
 * @param x 始点x座標
 * @param y 始点y座標
 * @param length 矢印の長さ
 * @param color 矢印の色
 * @param direction "right", "down", "left", "up"
 */
void Icon::drawArrow(int x, int y, int length, int color, const char* direction) {
    int dir = 0;
    if (strcmp(direction, "right") == 0) dir = 0;
    else if (strcmp(direction, "down") == 0) dir = 1;
    else if (strcmp(direction, "left") == 0) dir = 2;
    else if (strcmp(direction, "up") == 0) dir = 3;
    else return; // 無効な向き

    int shaft_x1 = x, shaft_y1 = y;
    int shaft_x2 = x, shaft_y2 = y;
    int head1_x, head1_y, head2_x, head2_y;
    int head_size = 5;

    switch (dir) {
        case 0: // 右
            shaft_x2 = x + length;
            head1_x = shaft_x2 - head_size;
            head1_y = y - head_size / 2;
            head2_x = shaft_x2 - head_size;
            head2_y = y + head_size / 2;
            break;
        case 1: // 下
            shaft_y2 = y + length;
            head1_x = x - head_size / 2;
            head1_y = shaft_y2 - head_size;
            head2_x = x + head_size / 2;
            head2_y = shaft_y2 - head_size;
            break;
        case 2: // 左
            shaft_x2 = x - length;
            head1_x = shaft_x2 + head_size;
            head1_y = y - head_size / 2;
            head2_x = shaft_x2 + head_size;
            head2_y = y + head_size / 2;
            break;
        case 3: // 上
            shaft_y2 = y - length;
            head1_x = x - head_size / 2;
            head1_y = shaft_y2 + head_size;
            head2_x = x + head_size / 2;
            head2_y = shaft_y2 + head_size;
            break;
    }

    canvas.drawLine(shaft_x1, shaft_y1, shaft_x2, shaft_y2, color);
    canvas.drawLine(shaft_x2, shaft_y2, head1_x, head1_y, color);
    canvas.drawLine(shaft_x2, shaft_y2, head2_x, head2_y, color);
}

void Icon::drawCommonIcon(const char *label, int x, int y, int w, int h, int r, int color) {
  canvas.fillRoundRect(x, y, w, h, r, color); 
  canvas.drawString(label, x + w / 2, y + h / 2);                 
}