## AppTemplate（テンプレート） - 簡潔

最終更新日: 2026年2月1日

### 🎯 役割
新しいアプリを作る時の **雛形（テンプレート）** です。
これをコピーして使うことで、基本構造ができた状態からスタートできます。

### 📁 ファイル
- `AppTemplate.h` - クラス定義（雛形）
- `AppTemplate.cpp` - 最小限の実装

---

## 🚀 使い方（新しいアプリの作り方）

### ステップ1: フォルダをコピー

```powershell
# PowerShellで実行
Copy-Item -Recurse AppTemplete AppMyNewApp
```

### ステップ2: ファイル名を変更

- `AppTemplate.h` → `AppMyNewApp.h`
- `AppTemplate.cpp` → `AppMyNewApp.cpp`

### ステップ3: クラス名を置換

ファイル内の `AppTemplate` を `AppMyNewApp` に一括置換します。

**AppMyNewApp.h**:
```cpp
#pragma once
#include "App/App.h"

class AppMyNewApp : public App {
public:
  void setup() override;
  void loop() override;
  void draw(M5Canvas &canvas) override;
  const char* appName() const override { return "MyNewApp"; }
};
```

### ステップ4: 機能を実装

**AppMyNewApp.cpp**:
```cpp
#include "AppMyNewApp.h"

void AppMyNewApp::setup() {
  // ボタンやスライダーを作る
}

void AppMyNewApp::loop() {
  // ボタンの状態更新など
}

void AppMyNewApp::draw(M5Canvas &canvas) {
  canvas.setTextSize(2);
  canvas.drawString("My New App", 50, 100);
  // ボタンやスライダーを描画
}
```

### ステップ5: AppManagerに登録

**main.cpp**:
```cpp
#include "App/AppMyNewApp/AppMyNewApp.h"

void setup() {
  M5.begin();
  appManager.registerApp(new AppMyNewApp());  // ★追加
  appManager.initializeApps();
  appManager.showHomeScreen();
}
```

---

## 📖 テンプレートに含まれるもの

- ✅ `App` クラスの継承
- ✅ 必須メソッドの宣言（`setup`, `loop`, `draw`）
- ✅ アプリ名の実装
- ✅ アイコン描画の基本実装

---

## 💡 カスタマイズのヒント

### アイコンの色を変える

```cpp
uint16_t iconBackgroundColor() const override {
  return TFT_BLUE;  // 青いアイコン
}

uint16_t iconPressedColor() const override {
  return TFT_DARKBLUE;  // 押した時は濃い青
}
```

### タッチイベントを追加

```cpp
void handleTouch(int16_t x, int16_t y) override {
  Serial.print("タッチされました: ");
  Serial.print(x);
  Serial.print(", ");
  Serial.println(y);
}
```

---

## 🎓 初心者向けまとめ

1. **このフォルダ自体は動かない**
   - あくまで「雛形」なので、コピーして使う

2. **コピー→名前変更→実装**
   - この3ステップで新しいアプリができる

3. **最初はシンプルに**
   - まず文字を表示するだけから始める
   - 慣れたらボタンやスライダーを追加

4. **既存のアプリを参考に**
   - `AppInfo` や `AppMotor` を見て学ぶ

---

## 📝 チェックリスト

新しいアプリを作る時のチェックリスト:

- [ ] フォルダをコピーした
- [ ] ファイル名を変更した
- [ ] クラス名を変更した
- [ ] `appName()` を変更した
- [ ] `setup()` を実装した
- [ ] `loop()` を実装した
- [ ] `draw()` を実装した
- [ ] `main.cpp` に `#include` を追加した
- [ ] `registerApp()` で登録した
- [ ] コンパイルが通った
- [ ] ホーム画面にアイコンが表示された

全部チェックできたら完成です！🎉
