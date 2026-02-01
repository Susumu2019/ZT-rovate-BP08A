## AppMotor（簡潔） - モーター制御

最終更新日: 2026年2月1日

### 🎯 役割
モーターを制御する画面です。速度調整、方向切り替えなどのUIを提供します。

### 📁 ファイル
- `AppMotor.h` - クラス定義
- `AppMotor.cpp` - モーター制御UIの実装

### 使い方
```cpp
appManager.registerApp(new AppMotor());
```

### 💡 実装例
スライダーとボタンでモーターを制御:
```cpp
void AppMotor::setup() {
  speedSlider = SliderBar(50, 100, 200, 30, 0, 255, 0);
  speedSlider.setOnChange([](int speed){
    // モーター速度を変更
    motorSetSpeed(speed);
  });
}
```

### ⚠️ 注意
- ハードウェアのピン設定を確認
- モータードライバとの通信方法（PWM、I2Cなど）を実装
- 安全のため、緊急停止ボタンを推奨
