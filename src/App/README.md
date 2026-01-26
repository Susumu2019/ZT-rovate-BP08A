## App - アプリケーション（簡潔）

### 🎯 このフォルダの目的

マイコンで **複数の画面や機能** を持つアプリを作る時、各機能を **独立したモジュール** として分けると管理しやすくなります。
このフォルダには、各「画面」や「アプリ機能」が入っています。

💡 **スマホと同じ**: スマホの「設定アプリ」「カメラアプリ」のように、機能ごとに分けているイメージです。

---

## 📂 サブディレクトリ一覧

| フォルダ | 役割 | いつ使う？ |
|---------|------|-----------|
| `AppInfo/` | アプリ情報表示 | バージョン情報やクレジット表示 |
| `AppLock/` | ロック画面 | 起動時のロック解除 |
| `AppManager/` | アプリ管理 | 複数アプリの切り替え制御（重要！） |
| `AppMotor/` | モーター制御 | モーターを使う場合 |
| `AppTemplete/` | テンプレート | 新しいアプリを作る時の雛形 |
| `HomeScreen/` | ホーム画面 | アプリ一覧を表示 |

---

## 📱 アプリの基本構造（App.h）

`App.h` は **全てのアプリの親クラス** です。すべてのアプリがこれを継承します。

### 必須メソッド（必ず実装）

```cpp
class MyApp : public App {
public:
  // 初期化（起動時に1回だけ実行）
  void setup() override {
    // ボタンを作る、変数を初期化するなど
  }
  
  // メインループ（繰り返し実行）
  void loop() override {
    // アプリのロジック
  }
  
  // 描画処理
  void draw(M5Canvas& canvas) override {
    // 画面に何を表示するか
  }
  
  // アプリ名
  const char* appName() const override {
    return "マイアプリ";
  }
};
```

### オプションのメソッド

```cpp
// タッチイベント
void handleTouch(int16_t x, int16_t y) override {
  // タッチされた時の処理
}

// ホーム画面のアイコン描画
void drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) override {
  // カスタムアイコンを描く
}

// アイコンの色設定
uint16_t iconBackgroundColor() const override { return TFT_BLUE; }
uint16_t iconPressedColor() const override { return TFT_DARKBLUE; }
```

---

## 🔄 AppManager の仕組み

`AppManager` は **複数のアプリを管理** する重要なクラスです。

### 主要機能

1. **アプリの登録**
   ```cpp
   AppManager manager;
   manager.registerApp(new AppInfo());  // 情報アプリを登録
   manager.registerApp(new AppMotor()); // モーターアプリを登録
   ```

2. **アプリの切り替え**
   ```cpp
   manager.switchToApp(0);  // 0番目のアプリに切り替え
   manager.switchToApp(1);  // 1番目のアプリに切り替え
   ```

3. **ホーム画面の表示**
   ```cpp
   manager.showHomeScreen();  // アプリ一覧を表示
   ```

4. **メインループ**
   ```cpp
   void loop() {
     manager.loop();   // 現在のアプリのloop()を実行
     
     M5.Canvas.clear();
     manager.draw(M5.Canvas);  // 現在のアプリを描画
     M5.Canvas.pushSprite(0, 0);
   }
   ```

💡 **自動管理**: AppManagerが現在のアプリを覚えておいてくれます。

---

## 🚀 新しいアプリの作り方

### ステップ1: AppTempleteをコピー

```powershell
# AppTempleteフォルダをコピーして名前を変更
Copy-Item AppTemplete AppMyNew
```

### ステップ2: ファイル名を変更

- `AppTemplate.h` → `AppMyNew.h`
- `AppTemplate.cpp` → `AppMyNew.cpp`

### ステップ3: クラス名を変更

```cpp
// AppMyNew.h
class AppMyNew : public App {
public:
  void setup() override;
  void loop() override;
  void draw(M5Canvas &canvas) override;
  const char* appName() const override { return "新アプリ"; }
};
```

### ステップ4: 機能を実装

```cpp
// AppMyNew.cpp
void AppMyNew::setup() {
  // 初期化処理
}

void AppMyNew::loop() {
  // メインロジック
}

void AppMyNew::draw(M5Canvas &canvas) {
  // 描画処理
  canvas.drawString("新しいアプリです", 50, 100);
}
```

### ステップ5: AppManagerに登録

```cpp
// main.cpp
#include "App/AppMyNew/AppMyNew.h"

AppManager manager;

void setup() {
  M5.begin();
  manager.registerApp(new AppMyNew());  // ★登録
  manager.initializeApps();
}
```

---

## 📖 完全なサンプル（ミニマルなアプリ）

```cpp
// SimpleApp.h
#pragma once
#include "App/App.h"

class SimpleApp : public App {
public:
  void setup() override {}
  
  void loop() override {}
  
  void draw(M5Canvas& canvas) override {
    canvas.setTextSize(2);
    canvas.drawString("Hello, World!", 50, 100);
  }
  
  const char* appName() const override {
    return "Simple";
  }
};
```

```cpp
// main.cpp で使う
manager.registerApp(new SimpleApp());
```

---

## 🎓 初心者向けまとめ

1. **各アプリは独立したクラス**
   - `App` クラスを継承して作る

2. **3つのメソッドは必須**
   - `setup()` : 初期化
   - `loop()` : メインロジック
   - `draw()` : 描画

3. **AppManagerが管理**
   - アプリの切り替え
   - ホーム画面表示
   - タッチイベント配信

4. **新しいアプリはテンプレートをコピー**
   - `AppTemplete/` を複製して使う

---

## 💡 設計の理念（なぜこの構成？）

### 分離の原則
各アプリは **他のアプリを知らない** ように設計されています。
- メリット: 1つのアプリを修正しても他に影響しない
- メリット: 別のプロジェクトに簡単に移植できる

### 継承による統一
全てのアプリが `App` クラスを継承することで:
- 同じインターフェースで扱える
- AppManagerが統一的に管理できる
- 新しいアプリを追加しやすい

### 責務の分担
- **App (各アプリ)**: 個別の機能実装
- **AppManager**: アプリの管理・切り替え
- **HomeScreen**: アプリ一覧表示

💡 **単一責任の原則**: 各クラスが1つの責務だけを持つことで、理解しやすく保守しやすいコードになります。

---

## ❓ よくある質問

### Q: アプリ間でデータを共有したい
**A**: グローバル変数か、AppManagerに共有データを持たせます。

```cpp
// グローバル変数の例
extern int sharedValue;  // App.h などで宣言
int sharedValue = 0;     // どこかの.cppで定義
```

### Q: アプリの実行順序は？
**A**: `registerApp()` で登録した順番になります。

### Q: アプリを削除したい
**A**: `registerApp()` の行をコメントアウトすればOK。

### Q: ホーム画面はカスタマイズできる？
**A**: `HomeScreen.cpp` を編集すればできます。

---

## 🔧 次のステップ

1. **既存のアプリを読む**
   - `AppInfo/` や `AppLock/` を見て構造を理解

2. **AppTempleteを使って作ってみる**
   - 簡単なアプリを1つ作成

3. **AppManagerを理解する**
   - アプリ切り替えの仕組みを学ぶ

各サブフォルダの README も参照してください！
