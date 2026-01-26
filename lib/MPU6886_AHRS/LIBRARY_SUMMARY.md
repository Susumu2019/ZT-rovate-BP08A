# MPU6886 AHRS Library - Summary

## 📦 Library Structure

```
lib/MPU6886_AHRS/
├── MPU6886.h              # Low-level sensor driver header
├── MPU6886.cpp            # Sensor driver implementation
├── MadgwickAHRS.h         # AHRS filter header
├── MadgwickAHRS.cpp       # Filter implementation
├── MPU6886_AHRS.h         # High-level unified API header
├── MPU6886_AHRS.cpp       # Unified API implementation
├── README.md              # Full documentation
├── library.properties     # Arduino library metadata
└── examples/
    └── BasicOrientation/
        └── BasicOrientation.ino  # Example sketch
```

## 🎯 Files Overview

### Core Files (6 files total)

1. **MPU6886.h/cpp** (2 files)
   - Low-level MPU6886 sensor driver
   - Shared I2C bus support
   - Raw sensor access
   - Platform-independent

2. **MadgwickAHRS.h/cpp** (2 files)
   - Madgwick AHRS filter algorithm
   - Quaternion-based orientation
   - Real-time dt support
   - No platform dependencies

3. **MPU6886_AHRS.h/cpp** (2 files)
   - High-level unified interface
   - Auto-calibration
   - Simplified API
   - Single-call update

### Documentation & Examples

4. **README.md** - Complete usage guide
5. **library.properties** - Arduino library metadata
6. **examples/BasicOrientation.ino** - Example code

## ✨ Key Improvements

### From Old Code:
- ❌ 10+ files scattered across src/include/lib
- ❌ Platform-specific Wire.begin() in driver
- ❌ Hard-coded I2C initialization
- ❌ Separate calibration logic
- ❌ Manual dt calculation required

### To New Library:
- ✅ **6 core files** in single directory
- ✅ **Shared I2C bus** - pass any TwoWire object
- ✅ **Auto dt calculation** - just call update()
- ✅ **Built-in calibration** - calibrateGyro(200)
- ✅ **3-level API** - low/mid/high level access
- ✅ **Portable** - works on any Arduino-compatible board

## 🚀 Usage Comparison

### Old Code (Before):
```cpp
// 5 separate includes
#include "IMU_6886.h"
#include "MadgwickAHRS_Simple.h"

IMU_6886 imu;
MadgwickAHRS madgwick;
float gyroBiasX, gyroBiasY, gyroBiasZ;
uint32_t lastUpdateMicros;

void setup() {
  imu.Init(2, 1);  // Hard-coded pins
  
  // Manual calibration (10+ lines)
  for (int i = 0; i < 200; i++) { ... }
  
  madgwick.begin(100);
  madgwick.setGain(0.4);
  lastUpdateMicros = micros();
}

void loop() {
  imu.getAccelData(&ax, &ay, &az);
  imu.getGyroData(&gx, &gy, &gz);
  
  gx -= gyroBiasX;  // Manual bias correction
  gy -= gyroBiasY;
  gz -= gyroBiasZ;
  
  // Manual dt calculation (5 lines)
  uint32_t now = micros();
  float dt = (now - lastUpdateMicros) * 1e-6f;
  lastUpdateMicros = now;
  
  madgwick.updateIMU(gx, gy, gz, ax, ay, az, dt);
  
  roll = madgwick.getRoll();
  pitch = madgwick.getPitch();
  yaw = madgwick.getYaw();
}
```

### New Code (After):
```cpp
// 1 include
#include "MPU6886_AHRS.h"

MPU6886_AHRS imu;

void setup() {
  Wire.begin(2, 1);
  imu.begin(&Wire);       // Flexible I2C
  imu.calibrateGyro(200); // Auto calibration
}

void loop() {
  imu.update();  // Everything automatic!
  
  float roll = imu.getRoll();
  float pitch = imu.getPitch();
  float yaw = imu.getYaw();
}
```

**Result:** 50% less code, easier to use, more portable!

## 📋 Migration Guide

### Step 1: Copy Library
Copy entire `lib/MPU6886_AHRS/` folder to your new project's `lib/` directory.

### Step 2: Update Includes
```cpp
// Old
#include "IMU_6886.h"
#include "MadgwickAHRS_Simple.h"

// New
#include "MPU6886_AHRS.h"
```

### Step 3: Update Initialization
```cpp
// Old
IMU_6886 imu;
MadgwickAHRS madgwick;
imu.Init(2, 1);
madgwick.begin(100);

// New
MPU6886_AHRS imu;
Wire.begin(2, 1);  // Or Wire1.begin()
imu.begin(&Wire, 0x68, 100, 0.4);
```

### Step 4: Simplify Loop
```cpp
// Old
imu.getAccelData(&ax, &ay, &az);
imu.getGyroData(&gx, &gy, &gz);
// ... manual dt, bias correction ...
madgwick.updateIMU(...);
roll = madgwick.getRoll();

// New
imu.update();
float roll = imu.getRoll();
```

## 🔧 Platform Support

Works on any Arduino-compatible board:
- ✅ ESP32 / ESP8266
- ✅ Arduino Uno / Mega / Nano
- ✅ STM32
- ✅ Teensy
- ✅ RP2040 (Raspberry Pi Pico)
- ✅ Any board with Wire library

## 📄 License

MIT License - Free for commercial and personal use.

## 🙏 Credits

Based on original M5Stack MPU6886 driver and Madgwick AHRS algorithm.
