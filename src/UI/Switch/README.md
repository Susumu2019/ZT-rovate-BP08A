## Switch - トグルスイッチ

最終更新日: 2026年2月1日

### 🎯 このライブラリの目的

ON/OFFを切り替える機能（WiFi、Bluetooth、LEDなど）に使う **トグルスイッチ** を実装します。
スマホでよく見る「スライドして切り替え」のスイッチを、タッチ操作で簡単に使えます。

### 📁 ファイル構成

- **`ToggleSwitch.h`** : クラス定義と関数の宣言
- **`ToggleSwitch.cpp`** : スイッチの描画・アニメーション・タッチ処理の実装

💡 **アニメーション**: つまみが滑らかに動くアニメーション付きです！

---

## 🚀 基本的な使い方

### ステップ1: スイッチを作成

```cpp
#include "UI/Switch/ToggleSwitch.h"

// ToggleSwitch(x, y, 幅, 高さ, 初期状態, OFF色, ON色, つまみ色, 文字色, ONラベル, OFFラベル)
ToggleSwitch wifiSwitch(100, 100, 80, 40, false);
```

**簡単な作り方**（色とラベルはデフォルト使用）:
```cpp
ToggleSwitch wifiSwitch(100, 100, 80, 40);  // OFFでスタート
ToggleSwitch ledSwitch(100, 150, 80, 40, true);  // ONでスタート
```

**パラメータの意味**:
- `100, 100` : 位置（x=100px, y=100px）
- `80, 40` : サイズ（幅80px、高さ40px）
- `false` : 初期状態（false=OFF, true=ON）

---

### ステップ2: 描画と更新

```cpp
void loop() {
  wifiSwitch.update();  // タッチ状態を更新（アニメーションも）
  
  M5.Canvas.clear();
  wifiSwitch.draw(M5.Canvas);  // スイッチを描画
  M5.Canvas.pushSprite(0, 0);
  
  delay(10);
}
```

💡 **自動アニメーション**: `update()` を呼ぶだけで、つまみが滑らかに動きます。

---

## 📚 よく使う関数

### 状態の取得・設定

```cpp
// 現在の状態を取得（true=ON, false=OFF）
bool isOn = wifiSwitch.getValue();
if (isOn) {
  Serial.println("WiFiはONです");
} else {
  Serial.println("WiFiはOFFです");
}

// 状態を直接設定（プログラムから変更）
wifiSwitch.setValue(true);   // ONにする
wifiSwitch.setValue(false);  // OFFにする
```

---

### 状態が変わった時の処理（コールバック）

スイッチを切り替えた時に自動で処理を実行できます:

```cpp
wifiSwitch.setCallback([](bool isOn){
  if (isOn) {
    Serial.println("WiFiをONにしました");
    // WiFi.begin(ssid, password);  // WiFi接続開始
  } else {
    Serial.println("WiFiをOFFにしました");
    // WiFi.disconnect();  // WiFi切断
  }
});
```

💡 **コールバックの使い道**:
- WiFi/Bluetooth の ON/OFF
- LED の点灯/消灯
- モーターの起動/停止
- 機能の有効/無効切り替え

---

### タッチ判定（簡素化）

本テンプレートでは `TouchManager` を経由してタッチ座標を取得します:

```cpp
touchManager.update();
int touchX = touchManager.getX();
int touchY = touchManager.getY();
if (wifiSwitch.isTouched(touchX, touchY)) {
  Serial.println("スイッチがタッチされました");
}
```

通常は `wifiSwitch.update()` が内部で判定しますので、`update()` を呼ぶだけで十分です。

---

## 🎨 色とラベルのカスタマイズ

スイッチの色とラベルは作成時に指定できます:

```cpp
ToggleSwitch mySwitch(
  100, 100, 80, 40,    // 位置とサイズ
  false,               // 初期状態（OFF）
  TFT_DARKGREY,        // OFF時の背景色
  TFT_GREEN,           // ON時の背景色
  TFT_WHITE,           // つまみの色
  TFT_WHITE,           // 文字の色
  "オン",              // ON時のラベル
  "オフ"               // OFF時のラベル
);
```

💡 **日本語も使える**: ラベルは自由に変更可能です（"有効"/"無効"など）。

---

## 📖 完全なサンプルコード

```cpp
#include <M5CoreS3.h>
#include "UI/Switch/ToggleSwitch.h"

// WiFiスイッチ（初期状態はOFF）
ToggleSwitch wifiSwitch(100, 100, 80, 40, false, 
  0x8410, DARKCYAN, WHITE, WHITE, "ON", "OFF");

void setup() {
  M5.begin();
  Serial.begin(115200);
  
  // スイッチが切り替わったら実行
  wifiSwitch.setCallback([](bool isOn){
    if (isOn) {
      Serial.println("WiFi ON");
      // ここにWiFi接続処理を書く
    } else {
      Serial.println("WiFi OFF");
      // ここにWiFi切断処理を書く
    }
  });
}

void loop() {
  wifiSwitch.update();  // 必ず毎フレーム呼ぶ
  
  M5.Canvas.clear();
  M5.Canvas.setTextSize(2);
  M5.Canvas.drawString("WiFi Settings", 80, 50);
  
  wifiSwitch.draw(M5.Canvas);
  
  // 現在の状態を表示
  String status = wifiSwitch.getValue() ? "接続中" : "切断中";
  M5.Canvas.drawString(status, 100, 160);
  
  M5.Canvas.pushSprite(0, 0);
  delay(10);
}
```

---

## 🎓 初心者向けまとめ

1. **スイッチは2状態を切り替える**:
   - ON (true) ⇔ OFF (false)

2. **基本は3ステップ**:
   - 作成 → `update()` → `draw()`

3. **状態の確認は `getValue()`**
   - true なら ON、false なら OFF

4. **切り替わった時の処理は `setCallback()`**
   - WiFi、LED、モーターなどの制御に便利

5. **アニメーション自動**:
   - `update()` を呼ぶだけで滑らかに動く

6. **用途**:
   - 設定画面の ON/OFF 項目
   - 機能の有効/無効切り替え
   - デバイスの接続管理

---

## ⚠️ よくある間違い

### ❌ 間違い: コールバックで `setValue()` を呼ぶ

```cpp
// これは無限ループになる！
mySwitch.setCallback([](bool isOn){
  mySwitch.setValue(!isOn);  // ❌ ダメ！
});
```

💡 **正しい方法**: コールバック内では外部の処理（WiFi、LEDなど）だけを行う。

### ✅ 正しい例

```cpp
mySwitch.setCallback([](bool isOn){
  if (isOn) {
    digitalWrite(LED_PIN, HIGH);  // ✅ 外部デバイスを制御
  } else {
    digitalWrite(LED_PIN, LOW);
  }
});
```
