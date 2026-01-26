## AppManager（簡潔）

### 🎯 このクラスの目的

スマホのように **複数のアプリを切り替えて使う** ために、AppManagerがアプリの管理を担当します。
- アプリの登録・起動・終了
- ホーム画面の表示
- タッチイベントの配信
- 画面遷移の制御

💡 **スマホのホーム画面**: ホームボタンを押すとアプリ一覧が出て、タップすると起動する、あの仕組みと同じです。

---

## 📁 ファイル構成

- **`AppManager.h`** : クラス定義と関数の宣言
- **`AppManager.cpp`** : アプリ管理ロジックの実装

---

## 🚀 基本的な使い方

### ステップ1: AppManagerを作成

```cpp
#include "App/AppManager/AppManager.h"

AppManager appManager;  // グローバルで1つだけ作る
```

💡 **シングルトン**: 通常、AppManagerは1つだけ作ります。

---

### ステップ2: アプリを登録

```cpp
#include "App/AppInfo/AppInfo.h"
#include "App/AppMotor/AppMotor.h"

void setup() {
  M5.begin();
  
  // アプリを登録（登録順にIDが割り当てられる）
  appManager.registerApp(new AppInfo());   // ID: 0
  appManager.registerApp(new AppMotor());  // ID: 1
  
  // アプリを初期化
  appManager.initializeApps();
  
  // ホーム画面を表示
  appManager.showHomeScreen();
}
```

💡 **`new`**: 動的にメモリを確保してオブジェクトを作ります。AppManagerが削除も管理してくれます。

---

### ステップ3: メインループで実行

```cpp
void loop() {
  // 現在のアプリのloop()を実行
  appManager.loop();
  
  // 描画
  M5.Canvas.clear();
  appManager.draw(M5.Canvas);
  M5.Canvas.pushSprite(0, 0);
  
  delay(10);
}
```

---

## 📚 主要な関数

### アプリの登録と初期化

```cpp
// アプリを登録
void registerApp(App* app);

// 全アプリの setup() を呼ぶ
void initializeApps();
```

**使用例**:
```cpp
appManager.registerApp(new MyApp1());
appManager.registerApp(new MyApp2());
appManager.initializeApps();  // MyApp1とMyApp2のsetup()が呼ばれる
```

---

### アプリの切り替え

```cpp
// 指定したIDのアプリに切り替え
void switchToApp(int appId);

// ホーム画面を表示
void showHomeScreen();

// 現在のアプリIDを取得
int getCurrentAppId() const;
```

**使用例**:
```cpp
// 0番目のアプリに切り替え
appManager.switchToApp(0);

// ホーム画面に戻る
appManager.showHomeScreen();

// 現在どのアプリが動いているか確認
int currentId = appManager.getCurrentAppId();
Serial.print("現在のアプリID: ");
Serial.println(currentId);
```

💡 **appId**: `registerApp()` で登録した順番（0から始まる）

---

### メインループと描画

```cpp
// 現在のアプリの loop() を実行
void loop();

// 現在のアプリを描画
void draw(M5Canvas &canvas);
```

**使用例**:
```cpp
void loop() {
  appManager.loop();   // 現在のアプリのメインロジック実行
  
  M5.Canvas.clear();
  appManager.draw(M5.Canvas);  // 現在のアプリを描画
  M5.Canvas.pushSprite(0, 0);
}
```

---

### タッチイベント

AppManagerはタッチイベントを現在のアプリに配信します:

```cpp
// タッチイベント（1回だけ）
void onTouch(int x, int y);

// 押した瞬間
void onPress(int x, int y);

// ドラッグ中
void onMove(int x, int y);

// 離した瞬間
void onRelease(int x, int y);
```

**使用例**:
```cpp
void loop() {
  // TouchManagerから取得したタッチ情報を渡す
  if (touchManager.wasPressed()) {
    appManager.onPress(touchManager.getX(), touchManager.getY());
  }
  
  appManager.loop();
  // ... 描画 ...
}
```

💡 **自動配信**: AppManagerが現在のアプリに自動的にタッチイベントを渡してくれます。

---

## 🏠 ホーム画面の仕組み

`showHomeScreen()` を呼ぶと、特別な「ホーム画面アプリ」が起動します。
ホーム画面では:
- 登録された全アプリのアイコンを表示
- タップするとそのアプリに切り替え

```cpp
// ホーム画面に戻る
appManager.showHomeScreen();
```

💡 **実装**: `HomeScreen` クラスが担当しています（`App/HomeScreen/`）

---

## 📖 完全なサンプルコード

```cpp
#include <M5CoreS3.h>
#include "App/AppManager/AppManager.h"
#include "App/AppInfo/AppInfo.h"
#include "App/AppLock/AppLock.h"
#include "system/touch/TouchManager.h"

AppManager appManager;

void setup() {
  M5.begin();
  Serial.begin(115200);
  
  // アプリを登録
  appManager.registerApp(new AppInfo());
  appManager.registerApp(new AppLock());
  
  // 初期化
  appManager.initializeApps();
  
  // ホーム画面から開始
  appManager.showHomeScreen();
}

void loop() {
  // タッチ状態を更新
  touchManager.update();
  
  // タッチイベントを配信
  if (touchManager.wasPressed()) {
    appManager.onPress(touchManager.getX(), touchManager.getY());
  }
  if (touchManager.isTouched()) {
    appManager.onMove(touchManager.getX(), touchManager.getY());
  }
  if (touchManager.wasReleased()) {
    appManager.onRelease(touchManager.getX(), touchManager.getY());
  }
  
  // アプリのメインロジック実行
  appManager.loop();
  
  // 描画
  M5.Canvas.clear();
  appManager.draw(M5.Canvas);
  M5.Canvas.pushSprite(0, 0);
  
  delay(10);
}
```

---

## 🎓 初心者向けまとめ

1. **AppManagerは1つだけ作る**
   - グローバル変数として定義

2. **アプリは登録順にID割り当て**
   - 0番、1番、2番...

3. **初期化の流れ**
   - `registerApp()` → `initializeApps()` → `showHomeScreen()`

4. **メインループは単純**
   - タッチイベント配信 → `loop()` → `draw()`

5. **ホーム画面に戻るには**
   - `showHomeScreen()` を呼ぶ

---

## 💡 内部の仕組み（興味がある人向け）

### アプリの保持方法

```cpp
std::vector<App*> apps;  // 登録されたアプリのリスト
App* currentApp;         // 現在実行中のアプリ
int currentAppId;        // 現在のアプリID
```

### 切り替え時の動作

`switchToApp(id)` が呼ばれると:
1. `currentApp` を `apps[id]` に変更
2. `currentAppId` を `id` に更新
3. 次の `loop()` や `draw()` では新しいアプリが実行される

💡 **シンプル**: ポインタを切り替えるだけなので高速です。

---

## ❓ よくある質問

### Q: アプリを動的に追加/削除できる？
**A**: 追加は `registerApp()` でいつでもできます。削除は現在未対応ですが、実装は可能です。

### Q: アプリ間でデータを共有したい
**A**: グローバル変数か、AppManagerにデータメンバを追加して共有します。

```cpp
// AppManager.h に追加
class AppManager {
public:
  int sharedData = 0;  // 共有データ
  // ...
};

// 各アプリから
extern AppManager appManager;
int value = appManager.sharedData;
```

### Q: アプリの起動順序は？
**A**: `registerApp()` で登録した順番になります。

### Q: ホーム画面をカスタマイズしたい
**A**: `App/HomeScreen/HomeScreen.cpp` を編集してください。

---

## 🔧 拡張のヒント

### 戻るボタンを追加したい

```cpp
// 各アプリに戻るボタンを置く
CoreS3Buttons backBtn("戻る", 10, 10, 60, 30, TFT_GREY, TFT_DARKGREY, TFT_WHITE);

backBtn.setCallback([](){
  appManager.showHomeScreen();  // ホーム画面に戻る
}, EVENT_TYPE::CLICK);
```

### アプリ起動時にパラメータを渡したい

```cpp
// App.h に追加
class App {
public:
  virtual void onActivate(void* param = nullptr) {}
};

// AppManager.cpp で呼ぶ
void AppManager::switchToApp(int appId, void* param) {
  currentApp = apps[appId];
  currentApp->onActivate(param);
}
```

---

## 📚 関連ドキュメント

- [`App/README.md`](../README.md) - アプリ全体の設計
- [`App/HomeScreen/README.md`](../HomeScreen/README.md) - ホーム画面の実装
- [`App/AppTemplete/README.md`](../AppTemplete/README.md) - 新しいアプリの作り方
