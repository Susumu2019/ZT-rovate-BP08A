## CoreS3 ボタンライブラリ

最終更新日: 2026年2月1日

このディレクトリには CoreS3 向けのシンプルなボタン UI 実装が含まれています。M5CoreS3 のタッチ/描画基盤上で動作するよう設計されています。

### 🎯 このライブラリの目的

マイコンでボタンを実装するには、以下のような処理が必要です：
1. 画面にボタンを描画する
2. タッチされたか検出する
3. タッチ座標がボタンの範囲内か判定する
4. 押した時・離した時・長押しした時の処理を実行する

これらを **毎回1から書くのは大変** なので、このライブラリで **簡単に使える部品** として提供しています。

### 📁 含まれるファイルとその役割

#### `Button.h` - ボタンの設計図
- **役割**: ボタンクラス（`CoreS3Buttons`）と管理クラス（`ButtonManager`）の「設計図」
- **何が書いてある？**:
  - どんな関数が使えるか（関数の宣言）
  - ボタンが持つデータ（位置、サイズ、色、状態など）
  - イベントの種類（クリック、長押しなど）

#### `ButtonTouch.cpp` - タッチ検出の実装
- **役割**: タッチされたかを判定し、イベントを発火させる処理
- **具体的には**:
  - タッチ座標を読み取る
  - ボタンの範囲内か判定する
  - 押した→離した→クリック、のような状態遷移を管理する
  - 長押し時間を計測する

**なぜ分けるの？**: タッチ処理と描画処理を分けることで、それぞれを独立してテスト・修正できます。

#### `ButtonView.cpp` - 描画の実装
- **役割**: ボタンを画面に描画する処理
- **具体的には**:
  - 背景色を塗る
  - 文字（ラベル）を描く
  - 押されている時の色に変える
  - アイコンを描く（オプション）

**なぜ分けるの？**: 見た目を変えたい時に、タッチ処理に影響を与えずに修正できます。

---

## 🚀 基本的な使い方（初心者向け）

### ステップ1: ヘッダをインクルード

まず、ボタンを使うためのファイルを読み込みます。

## CoreS3 ボタンライブラリ（簡素化版）

このディレクトリには CoreS3 向けのシンプルなボタン UI 実装が含まれます。
プロジェクトの簡素化に合わせ、ボタンは基本的に **押下（PRESS）／離上（RELEASE）／クリック（CLICK）** のみを想定しています。

簡潔な使い方
--
1. `#include "UI/Button/Button.h"`
2. `CoreS3Buttons btn("OK", x, y, w, h, bg, pressed, text);`
3. `setup()` 内で `btn.begin();` を呼ぶ（必要に応じて）
4. `loop()` 内で `btn.update();` と描画を呼ぶ

イベント
--
- `PRESS` : 押した瞬間
- `CLICK` : 押して離した時（一般的な選択肢）
- `RELEASE` : 離した瞬間
- `PRESSING` / `LONG_PRESS` は旧実装の名残として API は残していますが、デフォルトでは複雑な長押し判定は簡素化されています。使う場合は自前で実装を確認してください。

注意
--
- 長押しの精密な挙動は簡素化されているため、長押しを多用する UI の場合は専用実装を検討してください。

参照
--
- 実装: `Button.h`, `ButtonTouch.cpp`, `ButtonView.cpp`

  btn.update();
  M5.Canvas.clear();           // 画面クリア
  btn.draw(M5.Canvas);         // ボタン描画
  M5.Canvas.pushSprite(0, 0);  // ★これを忘れずに！
}
```

💡 **`pushSprite` って何？**: メモリ上に描いた絵を実際の画面に反映させる命令です。

---

### Q2: ボタンを押しても反応しない
**考えられる原因**:
1. `update()` を呼んでいない
2. コールバックを登録していない
3. ボタンの位置とタッチ座標がずれている

**解決方法**:
```cpp
void setup() {
  btn.begin();
  // ★コールバックを忘れずに登録！
  btn.setCallback([](){
    Serial.println("クリックされました");
  }, EVENT_TYPE::CLICK);
}

void loop() {
  btn.update();  // ★毎回呼ぶ！
  // ... 描画処理 ...
}
```

---

### Q3: 長押しの判定時間を変えたい
**デフォルトでは約1秒** ですが、変更できます:

```cpp
btn.setLongPressMs(2000);  // 2秒に変更
btn.setOnLongPress([](){
  Serial.println("2秒長押しされました");
});
```

---

### Q4: ボタンの位置やサイズを後から変更したい
現状のライブラリには直接変更するメソッドがありません。

**対処法**:
1. 新しいボタンを作り直す
2. または `Button.h` に `setPosition(x, y)` などのメソッドを追加する

---

### Q5: ボタンの色を動的に変えたい
`setColors()` メソッドを使います:

```cpp
// 通常は青、押すと濃い青
btn.setColors(TFT_BLUE, TFT_DARKBLUE, TFT_WHITE);

// 条件によって色を変える例
if (isError) {
  btn.setColors(TFT_RED, TFT_MAROON, TFT_WHITE);
}
```

---

## 🎓 初心者向けまとめ

1. **ボタンは3ステップで使える**:
   - 作成 → 初期化 → 更新＆描画

2. **コールバックで「押されたら○○」を実現**:
   - `setCallback()` でイベントを登録

3. **複数ボタンは ButtonManager で管理**:
   - `addButton()`, `updateAll()`, `drawAll()`

4. **困ったら**:
   - `update()` を毎フレーム呼んでいるか確認
   - `pushSprite()` で画面に反映しているか確認
   - シリアルモニタで `Serial.println()` でデバッグ

---

## 📖 参考: 完全なサンプルコード

```cpp
#include "UI/Button/Button.h"
#include <M5CoreS3.h>

CoreS3Buttons btnOK("OK", 50, 100, 100, 50, TFT_GREEN, TFT_DARKGREEN, TFT_WHITE);

void setup() {
  M5.begin();
  Serial.begin(115200);
  
  btnOK.begin();
  btnOK.setCallback([](){
    Serial.println("OKボタンがクリックされました！");
  }, EVENT_TYPE::CLICK);
}

void loop() {
  btnOK.update();
  
  M5.Canvas.clear();
  btnOK.draw(M5.Canvas);
  M5.Canvas.pushSprite(0, 0);
  
  delay(10);  // 10ms待機
}
```

このコードをコピーして試してみてください！

