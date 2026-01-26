#include "TouchManager.h"

TouchManager touchManager;

void TouchManager::update() {
    // Minimal touch update: read primary touch and provide simple edge flags.
    int count = CoreS3.Touch.getCount();
    if (count > 0) {
        auto detail = CoreS3.Touch.getDetail(0);
        bool pressed = detail.isPressed();
        touchInfo_.x = detail.x;
        touchInfo_.y = detail.y;
        touchInfo_.isPressed = pressed;
        touchInfo_.count = 1;
        touchInfo_.state = 0;
        // simple edge detection using prev_pressed_
        touchInfo_.wasPressed = (!prev_pressed_ && pressed);
        touchInfo_.wasReleased = (prev_pressed_ && !pressed);
        prev_pressed_ = pressed;
    } else {
        // No touch detected
        touchInfo_.x = 0;
        touchInfo_.y = 0;
        touchInfo_.isPressed = false;
        touchInfo_.count = 0;
        touchInfo_.state = 0;
        touchInfo_.wasPressed = (prev_pressed_ && false);
        touchInfo_.wasReleased = (prev_pressed_ && true);
        prev_pressed_ = false;
    }
}
