#include "Button.h"
#include <M5CoreS3.h>

void CoreS3Buttons::draw(M5Canvas &canvas) {
  // Draw button rectangle
  canvas.fillRoundRect(x_, y_, w_, h_, 5, pressed_ ? color_pressed_ : color_bg_);
  // Draw icon if present
  int label_x = x_ + w_ / 2;
  if (has_icon_) {
    int iconSize = h_ - 10;
    int ix = x_ + 8;
    int iy = y_ + (h_ - iconSize) / 2;
    // draw a rounded-square icon instead of plain rectangle
    int corner = std::max(4, iconSize / 6);
    canvas.fillRoundRect(ix, iy, iconSize, iconSize, corner, icon_color_);
    label_x = x_ + iconSize + 16 + (w_ - iconSize - 16) / 2; // roughly center remaining area
    // If a special icon type is set (e.g., Lock), draw it centered in the button instead of text
  }

  canvas.setFont(&fonts::Font2);
  canvas.setTextSize(1);
  canvas.setTextColor(color_text_);
  canvas.setTextDatum(MC_DATUM);
  canvas.drawString(label_.c_str(), label_x, y_ + h_ / 2);
}
