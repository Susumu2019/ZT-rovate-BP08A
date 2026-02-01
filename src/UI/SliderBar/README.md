## SliderBar - スライダーコンポーネント

最終更新日: 2026年2月1日

### 🎯 このライブラリの目的

音量調整、明るさ設定、速度調整など、**数値を直感的に変更したい** 時にスライダーが便利です。
このライブラリは、タッチで動かせるスライダーを簡単に実装できます。

### 📁 ファイル構成

- **`SliderBar.h`** : クラス定義と関数の宣言
- **`SliderBar.cpp`** : スライダーの描画とタッチ処理の実装

---

## 🚀 基本的な使い方

### ステップ1: スライダーを作成

```cpp
#include "UI/SliderBar/SliderBar.h"

// SliderBar(x, y, 幅, 高さ, 初期値, 最大値, 最小値, レール色, 塗りつぶし色, つまみ色, 押下つまみ色, 文字色)
SliderBar slider(50, 100, 200, 30, 50, 100, 0);
```

**簡単な作り方**（色はデフォルト使用）:
```cpp
SliderBar slider(50, 100, 200, 30);  // 初期値50、最大100、最小0
```

**パラメータの意味**:
- `50, 100` : 位置（x=50px, y=100px）
- `200, 30` : サイズ（幅200px、高さ30px）
- 初期値 : 50（起動時の値）
- 最大値 : 100
- 最小値 : 0

💡 **スライダーの値**: 0〜100の間を動かせます（範囲は変更可能）

---

### ステップ2: 描画と更新

```cpp
void loop() {
  slider.update();  // タッチ状態を更新
  
  M5.Canvas.clear();
  slider.draw(M5.Canvas);  // スライダーを描画
  M5.Canvas.pushSprite(0, 0);
  
  delay(10);
}
```

---

## 📚 よく使う関数

### 値の取得・設定

```cpp
// 現在の値を取得
int currentValue = slider.getValue();
Serial.print("現在の値: ");
Serial.println(currentValue);

// 値を直接設定（プログラムから変更）
slider.setValue(75);  // 75に設定

// 範囲を変更（最小値、最大値）
slider.setRange(0, 255);  // 0〜255の範囲に変更
```

---

### 値が変わった時の処理（コールバック）

スライダーを動かした時に自動で処理を実行できます:

```cpp
slider.setOnChange([](int newValue){
  // スライダーを動かすとここが実行される
  Serial.print("スライダーの値が変わりました: ");
  Serial.println(newValue);
  
  // 例: LEDの明るさを変える
  // ledcWrite(0, newValue);
});
```

💡 **コールバックの使い道**:
- 音量リアルタイム調整
- LED明るさ制御
- モーター速度調整
- グラフの表示範囲変更

---

### タッチ判定

特定の座標がスライダーに触れているか確認:

```cpp
// 本テンプレートでは TouchManager を経由してタッチ座標を得ます
touchManager.update();
int touchX = touchManager.getX();
int touchY = touchManager.getY();

if (slider.isTouched(touchX, touchY)) {
  Serial.println("スライダーがタッチされました");
}

// タッチ位置から値を設定
if (slider.isTouched(touchX, touchY)) {
  slider.setValueFromTouch(touchX);
}
```

💡 **普通は `update()` が自動でやってくれる**: 通常は自分で `isTouched()` を呼ぶ必要はありません。

---

## 🎨 色のカスタマイズ

スライダーの色は作成時に指定できます:

```cpp
SliderBar slider(
  50, 100, 200, 30,    // 位置とサイズ
  50, 100, 0,          // 初期値、最大、最小
  TFT_DARKGREY,        // レール（背景）の色
  TFT_BLUE,            // 塗りつぶし（左側）の色
  TFT_WHITE,           // つまみの色
  TFT_LIGHTGREY,       // 押した時のつまみの色
  TFT_WHITE            // 数値表示の文字色
);
```

💡 **色の定数**: `TFT_RED`, `TFT_BLUE`, `TFT_GREEN` など、M5Stackで定義済みの色が使えます。

---

## 📖 完全なサンプルコード

```cpp
#include <M5CoreS3.h>
#include "UI/SliderBar/SliderBar.h"

// 音量スライダー (0〜100)
SliderBar volumeSlider(50, 100, 220, 30, 50, 100, 0);

void setup() {
  M5.begin();
  Serial.begin(115200);
  
  // 値が変わったら実行する処理
  volumeSlider.setOnChange([](int volume){
    Serial.print("音量: ");
    Serial.println(volume);
  });
}

void loop() {
  volumeSlider.update();  // タッチ状態を更新
  
  M5.Canvas.clear();
  M5.Canvas.setTextSize(2);
  M5.Canvas.drawString("音量調整", 50, 50);
  
  volumeSlider.draw(M5.Canvas);
  
  M5.Canvas.pushSprite(0, 0);
  delay(10);
}
```

---

## 🎓 初心者向けまとめ

1. **スライダーは3ステップで使える**:
   - 作成 → `update()` → `draw()`

2. **値の取得は `getValue()`**

3. **値が変わった時の処理は `setOnChange()`**

4. **用途**:
   - 音量、明るさ、速度など数値調整全般
   - グラフの表示範囲
   - RGB値の調整

5. **カスタマイズ**:
   - 色は作成時に自由に変更可能
   - 範囲は `setRange()` で変更可能
