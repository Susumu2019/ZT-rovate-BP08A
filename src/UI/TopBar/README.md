## TopBar - 画面上部バー

### 🎯 このライブラリの目的


### 📁 ファイル構成



## 🚀 基本的な使い方

### ステップ1: TopBarオブジェクトを作成

```cpp
#include "UI/TopBar/TopBar.h"

TopBar topBar;  // トップバーオブジェクト
```


### ステップ2: 初期化

```cpp
void setup() {
  M5.begin();
   topBar.begin();  // 初期化
}
```


### ステップ3: タイトルを設定

```cpp
topBar.setTitle("タイトル");  // タイトル設定
```

💡 **いつでも変更可能**: 画面遷移時に `setTitle()` を呼ぶだけでOK


### ステップ4: 更新と描画

```cpp
void loop() {
  topBar.update();  // 状態を更新（時刻など）
  
  M5.Canvas.clear();
  topBar.draw(M5.Canvas);  // トップバーを描画
  
  // ここに他の画面要素を描画
  
  M5.Canvas.pushSprite(0, 0);
  delay(10);
}
```


## 📚 よく使う関数

### タイトルの設定

```cpp
// 現在のアプリ名などを表示
topBar.setTitle("設定");
topBar.setTitle("WiFi接続");
topBar.setTitle("モーター制御");
```

💡 **使い道**: 画面ごとに異なるタイトルを表示


### バーの高さを取得

```cpp
int barHeight = topBar.height();  // デフォルトは24ピクセル

// トップバーの下にコンテンツを配置
int contentY = barHeight + 10;  // バーの10px下から開始
```

💡 **レイアウト計算に便利**: 他のUIパーツの位置を決める時に使います


## 🎨 TopBarの特徴

### 自動表示される情報

TopBarは以下の情報を **自動的に表示** します:

1. **タイトル（左側）**
   - `setTitle()` で設定した文字列

2. **ライフインジケーター（右側）**
   - `_/|^-` の文字が回転してアニメーション
   - システムが正常に動作していることを示す

💡 **ライフインジケーター**: システムがフリーズしていないことを視覚的に確認できます


## 📖 完全なサンプルコード

```cpp
#include <M5CoreS3.h>
#include "UI/TopBar/TopBar.h"

TopBar topBar;

void setup() {
  M5.begin();
  topBar.begin();
  topBar.setTitle("サンプルアプリ");
}

void loop() {
  topBar.update();
  
  M5.Canvas.clear();
  
  // トップバーを描画
  topBar.draw(M5.Canvas);
  
  // トップバーの下にコンテンツを配置
  int contentY = topBar.height() + 10;
  M5.Canvas.setTextSize(2);
  M5.Canvas.drawString("メインコンテンツ", 10, contentY);
  
  M5.Canvas.pushSprite(0, 0);
  delay(10);
}
```


## 🎓 初心者向けまとめ

1. **TopBarは画面上部の共通バー**:
   - タイトル表示
   - システム状態表示

2. **基本は3ステップ**:
   - `begin()` で初期化 → `update()` で更新 → `draw()` で描画

3. **タイトルは `setTitle()` で変更**:
   - 画面遷移時に呼ぶだけ

4. **高さは `height()` で取得**:
   - レイアウト計算に使う

5. **用途**:
   - アプリの共通ヘッダ
   - 現在の画面名表示
   - システム状態の監視


## 🔧 カスタマイズのヒント

### 背景色や文字色を変更したい場合

現在のコードでは `TopBar.h` 内で色が定義されています:

```cpp
uint16_t bgColor_ = WHITE;     // 背景色
uint16_t textColor_ = BLACK;   // 文字色
```

**変更方法**:
1. `TopBar.h` を開く
2. `bgColor_` や `textColor_` の値を変更
3. 例: `uint16_t bgColor_ = TFT_BLUE;`

💡 **将来の改善案**: `setColors()` メソッドを追加して動的に変更できるようにする


### バッテリー残量や時刻を表示したい場合

`TopBar.cpp` の `draw()` 関数に以下を追加:

```cpp
// 時刻を表示（例）
canvas.drawString(getTimeString(), 200, 5);

// バッテリー残量を表示（例）
int battery = M5.Power.getBatteryLevel();
canvas.drawString(String(battery) + "%", 250, 5);
```

💡 **初心者へ**: まずは既存のコードをそのまま使って、慣れてからカスタマイズしましょう。


## ⚙️ 内部の仕組み（興味がある人向け）

### ライフインジケーターの仕組み

```cpp
char life_char_[6] = "_/|^-";  // アニメーション用文字
int life_counter_ = 0;          // カウンター
```

`update()` が呼ばれるたびに `life_counter_` が増え、配列の文字を順番に表示します。
これにより `_` → `/` → `|` → `^` → `-` と回転しているように見えます。

💡 **なぜ必要？**: マイコンがフリーズ（固まった）時に、この文字が止まることで気づけます。
