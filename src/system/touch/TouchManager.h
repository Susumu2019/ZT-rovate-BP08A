#pragma once
#include <M5CoreS3.h>

/**
 * @brief Centralized touch input manager
 * 
 * This class provides a single point of touch input reading from CoreS3.Touch.
 * All components should use TouchManager instead of directly calling CoreS3.Touch.
 */
class TouchManager {
public:
    struct TouchInfo {
        int x;
        int y;
        bool isPressed;
        bool wasPressed;
        bool wasReleased;
        int state;
        int count;
    };

    /**
     * @brief Update touch state - should be called once per frame
     */
    void update();

    /**
     * @brief Get current touch information
     */
    const TouchInfo& getTouchInfo() const { return touchInfo_; }

    // Convenience methods
    bool isTouched() const { return touchInfo_.isPressed; }
    bool wasPressed() const { return touchInfo_.wasPressed; }
    bool wasReleased() const { return touchInfo_.wasReleased; }
    int getX() const { return touchInfo_.x; }
    int getY() const { return touchInfo_.y; }
    int getCount() const { return touchInfo_.count; }
    int getState() const { return touchInfo_.state; }
private:
    TouchInfo touchInfo_;
    // simple previous-pressed flag for minimal edge detection
    bool prev_pressed_ = false;
};

extern TouchManager touchManager;
