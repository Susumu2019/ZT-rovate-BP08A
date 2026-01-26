# Touch Processing Architecture

## Overview
This document describes the centralized touch processing architecture implemented in the ShiotsukiProject_CoreS3.

## Architecture

### TouchManager (Centralized Touch Input)
- **Location**: `src/touch/TouchManager.h` and `src/touch/TouchManager.cpp`
- **Purpose**: Single point of touch input reading from CoreS3.Touch hardware
- **Pattern**: Singleton
- **Usage**: Call `TouchManager::getInstance().update()` once per frame in the main loop

### Touch Flow
1. **Main Loop** (`main.cpp`): CoreS3.update() updates hardware state
2. **AppManager** (`AppManager::loop()`): 
   - Calls `TouchManager::getInstance().update()` once per frame
   - Retrieves touch info from TouchManager
   - Forwards touch events to current app via handlePress/Move/Release
3. **Apps**: 
   - Receive touch events via handlePress/Move/Release methods
   - Forward to UI components as needed (e.g., SliderBar)
4. **UI Components**:
   - **Buttons**: Call `TouchManager::getInstance().getTouchInfo()` in update()
   - **SliderBar**: Receive touch coordinates via handlePress/Move/Release from apps

## Benefits
- Touch input read only once per frame (reduced I2C overhead)
- Single source of truth for touch state
- Easier to maintain and debug
- Reduced code duplication
- Better performance

## Migration Guide
If you need to add new touch-based UI components:

### DO:
- Use `TouchManager::getInstance().getTouchInfo()` to get touch state
- Call this in your update() method or similar periodic function

### DON'T:
- Call `CoreS3.Touch.getCount()` or `CoreS3.Touch.getDetail()` directly
- Read touch input multiple times per frame

### Example:
```cpp
void MyComponent::update() {
    const auto& touch = TouchManager::getInstance().getTouchInfo();
    
    if (touch.isPressed) {
        // Handle touch at (touch.x, touch.y)
    }
}
```

## Touch Info Structure
```cpp
struct TouchInfo {
    int x;              // Touch X coordinate
    int y;              // Touch Y coordinate
    bool isPressed;     // Currently touching
    bool wasPressed;    // Just started touching
    bool wasReleased;   // Just released touch
    int state;          // Raw touch state
    int count;          // Number of touch points
};
```
