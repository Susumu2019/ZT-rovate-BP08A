#pragma once
#include <M5CoreS3.h>
#include <functional>
#include <string>
#include <vector>

enum EVENT_TYPE {
  PRESS = 0,
  RELEASE,
  LONG_PRESS,
  PRESSING,
  CLICK
};

class CoreS3Buttons {
public:
  using Callback = std::function<void()>;
  CoreS3Buttons(const char* label, int x, int y, int w, int h,
                uint16_t color_bg , uint16_t color_pressed , uint16_t color_text = WHITE);

  void begin();
  void update();
  void draw(M5Canvas &canvas);
  // ...existing code...

  void setCallback(Callback cb, EVENT_TYPE trigger = EVENT_TYPE::CLICK);
  bool isPressed() const;

  void setColors(uint16_t bg, uint16_t pressed, uint16_t text);

  // simple icon support: draw a colored square at left of button
  void setIconColor(uint16_t color);
  // icon type (for special drawing e.g., lock)
  enum class IconType { None, Lock };
  void setIconType(IconType t);

  // Long-press support
  void setOnLongPress(Callback cb);
  void setLongPressMs(uint32_t ms);
  // set callback when touch is released (after a press inside)
  void setOnRelease(Callback cb);

private:
  int x_, y_, w_, h_;
  // Own the label string to avoid lifetime issues with callers
  std::string label_;
  bool pressed_;
  int prev_touch_state_;
  Callback callback_;

  // 色メンバ
  uint16_t color_bg_;
  uint16_t color_pressed_;
  uint16_t color_text_;

  // simple icon: colored square
  bool has_icon_;
  uint16_t icon_color_;
  IconType icon_type_;

  // 追加: 押してから離したかを判定するためのフラグ
  bool was_pressed_inside_;

  // Long-press related
  Callback longpress_callback_;
  // threshold millis to consider a long-press
  uint32_t long_press_ms_;
  // whether long-press already triggered for the current press
  bool long_press_triggered_;
  // millis when the press started
  uint32_t press_start_ms_;
  // optional release callback (called on release when press started inside)
  Callback release_callback_;
  // which event should invoke the primary callback_
  EVENT_TYPE callback_trigger_ = EVENT_TYPE::CLICK;
};

// Small manager class embedded in the header to avoid a separate translation unit.
// It keeps a vector of CoreS3Buttons and provides convenience methods used
// across the project: add, updateAll, drawAll and clear.
class ButtonManager {
public:
  void addButton(CoreS3Buttons &&b) { list_.emplace_back(std::move(b)); }
  void updateAll() { for (auto &b : list_) b.update(); }
  void drawAll(M5Canvas &canvas) { for (auto &b : list_) b.draw(canvas); }
  void clear() { list_.clear(); }

private:
  std::vector<CoreS3Buttons> list_;
};
