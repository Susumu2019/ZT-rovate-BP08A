## timer - ハードウェアタイマー

### 🎯 タイマーって何？（初心者向け）

**ハードウェアタイマー** は、マイコンが **正確な時間間隔** で処理を実行するための仕組みです。

**なぜ必要？**:

**使い道**:

💡 **割り込み**: タイマーは「割り込み」という仕組みで、メインプログラムを一時停止して処理を実行します。


## timer（簡潔）

このフォルダは周期処理を行うための簡単なタイマーラッパーを含みます。

使い方の流れ
- タイマーオブジェクトを作成
- `update()` を `loop()` 内で呼び、コールバックをトリガー

詳細は `timer.h` / `timer.cpp` のコメントを参照してください。
### グローバルオブジェクト

```cpp
extern PublicTimer publicTimer;  // どこからでも使える
```

### 初期化

```cpp
void setup() {
  M5.begin();
  publicTimer.begin();  // タイマー開始（10msごとに割り込み）
}
```

💡 **デフォルト**: 10ms（0.01秒）ごとに割り込みが発生します。

---

## 📚 使えるカウンター

このタイマーは複数の時間間隔でカウントします:

| カウンター | 間隔 | 用途 |
|-----------|------|------|
| `getCount10()` | 10ms | 高速な処理 |
| `getCount100()` | 100ms | 中速な処理 |
| `getCount500()` | 500ms | 点滅など |
| `getCount1000()` | 1000ms (1秒) | 時計、定期送信 |

### 使用例: 1秒ごとに処理

```cpp
void loop() {
  static int lastCount = 0;
  int currentCount = publicTimer.getCount1000();
  
  if (currentCount != lastCount) {
    // 1秒経過した
    Serial.println("1秒経ちました");
    lastCount = currentCount;
  }
}
```

---

## 📚 ワンショット（1回だけ）カウンター

「1回だけ」実行したい処理に便利:

```cpp
int count10One = publicTimer.getCount10One();
if (count10One > 0) {
  // 10ms後に1回だけ実行
  Serial.println("10ms経過");
}
```

💡 **自動リセット**: 読み取ると自動的に0に戻ります。

---

## 📚 点滅カウンター

LEDの点滅に便利な自動反転カウンター:

```cpp
// 100msごとに0/1が反転
bool blink = publicTimer.getFlicker100() % 2;
digitalWrite(LED_PIN, blink ? HIGH : LOW);

// 500msごとに反転
bool blink2 = publicTimer.getFlicker500() % 2;

// 1000msごとに反転
bool blink3 = publicTimer.getFlicker1000() % 2;
```

---

## 📖 完全なサンプルコード

### 例1: 1秒ごとにメッセージ表示

```cpp
#include <M5CoreS3.h>
#include "timer/timer.h"

void setup() {
  M5.begin();
  Serial.begin(115200);
  publicTimer.begin();
}

void loop() {
  static int lastCount = 0;
  int current = publicTimer.getCount1000();
  
  if (current != lastCount) {
    Serial.print("1秒経過: ");
    Serial.println(current);
    lastCount = current;
  }
}
```

### 例2: LED点滅（500ms間隔）

```cpp
#define LED_PIN 2

void setup() {
  M5.begin();
  pinMode(LED_PIN, OUTPUT);
  publicTimer.begin();
}

void loop() {
  // 500msごとに反転
  bool state = publicTimer.getFlicker500() % 2;
  digitalWrite(LED_PIN, state);
  
  delay(10);
}
```

### 例3: 10msごとにセンサー読み取り

```cpp
void loop() {
  static int lastCount = 0;
  int current = publicTimer.getCount10();
  
  if (current != lastCount) {
    // 10msごとに実行
    int sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    lastCount = current;
  }
}
```

---

## 🎓 初心者向けまとめ

1. **タイマーは正確な時間管理**
   - `delay()` と違って他の処理をブロックしない

2. **複数の時間間隔が使える**
   - 10ms, 100ms, 500ms, 1000ms

3. **カウンターを比較して処理**
   - 前回の値と比較して変化を検出

4. **点滅には Flicker カウンター**
   - 自動で0/1が切り替わる

---

## ⚙️ 内部の仕組み（興味がある人向け）

### 割り込みハンドラ (ISR)

タイマーが10msごとに `onPublicTimer()` を呼び出します:

```cpp
static void IRAM_ATTR onPublicTimer() {
  // ここが10msごとに実行される
  PublicTimer::instance().handleISR();
}
```

💡 **`IRAM_ATTR`**: 割り込み関数は高速なRAMに配置されます。

### カウンターの更新

```cpp
void PublicTimer::handleISR() {
  count_timer_10_++;        // 10msカウンタ増加
  if (count_timer_10_ >= 10) {
    count_timer_100_++;     // 100msカウンタ増加
    count_timer_10_ = 0;
  }
  // ... 以下同様
}
```

---

## ⚠️ 注意事項

### 割り込み内での制限

割り込みハンドラ内では以下ができません:
- ❌ `delay()`
- ❌ `Serial.println()`（デバッグ時は注意）
- ❌ 長時間かかる処理

💡 **フラグだけ立てる**: 割り込み内ではフラグを立てて、`loop()` で処理するのが安全です。

### シングルトンパターン

```cpp
PublicTimer& instance();  // インスタンスは1つだけ
```

複数のタイマーオブジェクトは作れません。グローバルな `publicTimer` を使ってください。

---

## 🔧 カスタマイズのヒント

### 割り込み周期を変更

`timer.cpp` の `begin()` で変更:
```cpp
void PublicTimer::begin() {
  timer_ = timerBegin(0, 80, true);  // 80で1MHz (1μs単位)
  timerAttachInterrupt(timer_, &onPublicTimer, true);
  timerAlarmWrite(timer_, 10000, true);  // 10000μs = 10ms
  timerAlarmEnable(timer_);
}
```

💡 **5msにしたい**: `10000` を `5000` に変更

---

## 📚 関連リソース

- [ESP32 Hardware Timer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/timer.html)
- [Arduino タイマー割り込み入門](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)
