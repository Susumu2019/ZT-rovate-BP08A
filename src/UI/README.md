
## UI部品（概要）

最終更新日: 2026年2月16日  
対応バージョン: v1.0.0

このフォルダは再利用可能な画面部品（UIコンポーネント）を収めています。

### 基本的な使い方

1. ヘッダを `#include` する
2. オブジェクトを生成（例: `CoreS3Buttons btn(...)`）
3. `setup()` で初期化
4. `loop()` で `update()`、描画は `draw(canvas)`

#### ポイント
- タッチ情報は `touchManager` 経由で取得
- ボタン等は `PRESS/CLICK/RELEASE` を想定

---

### 主な部品

- [Button/](Button/README.md) : ボタンUI
- [SliderBar/](SliderBar/README.md) : スライダーUI
- [Switch/](Switch/README.md) : トグルスイッチUI
- [Icon/](Icon/README.md) : アイコン描画

---

### 参考

- 詳細は各サブフォルダのREADME参照

### 📱 TopBar（上部バー）
**何ができる？**
- 画面タイトルの表示
- システム状態の監視
- 共通ヘッダとして全画面で使用

**おすすめ度**: ⭐⭐⭐ アプリらしくなる

📖 詳細は [`TopBar/README.md`](TopBar/README.md) へ

---

## 🎓 初心者向け学習順序

### 1. まずはButtonから（30分）
ボタンが一番シンプルで分かりやすいです。
- ボタンを作る → 描画する → クリックを検出する

### 2. 次にTopBarを追加（15分）
画面にタイトルをつけてアプリらしくします。

### 3. SliderBarで数値調整（30分）
スライダーでLEDの明るさを変えてみましょう。

### 4. Switchで機能切り替え（20分）
ON/OFFスイッチで機能を制御します。

### 5. Iconで装飾（10分）
矢印やアイコンで見た目を良くします。

---

## 💡 設計の哲学（なぜこの構成？）

### 独立性
各コンポーネントは **他に依存せず独立** しています。
- ボタンだけ使いたい → Button だけ include すればOK
- 全部使いたい → 全部 include してもOK

### 一貫性
すべてのコンポーネントが **同じパターン** で使えます:
1. オブジェクト作成
2. `update()` で状態更新
3. `draw()` で描画

💡 **覚えることが少ない**: 1つ覚えれば、他も同じように使えます。

### 拡張性
新しいコンポーネントも **同じパターンで追加** できます:
- 新しいフォルダを作る
- `.h` と `.cpp` を作る
- `update()` と `draw()` を実装

---

## 🔧 カスタマイズのヒント

### 色を変えたい
各コンポーネントは作成時に色を指定できます:
```cpp
CoreS3Buttons btn("OK", 10, 10, 100, 40, 
  TFT_RED,      // 通常の背景色
  TFT_DARKRED,  // 押した時の色
  TFT_WHITE     // 文字色
);
```

### 新しい部品を作りたい
既存のコンポーネントを参考に:
1. `Button/` をコピーして名前を変更
2. 必要な機能を追加
3. 同じパターン（`update()`, `draw()`）を維持

---

## 📖 サンプル: 全部使った画面

```cpp
#include <M5CoreS3.h>
#include "UI/TopBar/TopBar.h"
#include "UI/Button/Button.h"
#include "UI/SliderBar/SliderBar.h"
#include "UI/Switch/ToggleSwitch.h"

TopBar topBar;
CoreS3Buttons btn("実行", 50, 180, 100, 40, TFT_GREEN, TFT_DARKGREEN, TFT_WHITE);
SliderBar slider(50, 80, 220, 30, 50, 100, 0);
ToggleSwitch wifiSwitch(50, 130, 80, 30);

void setup() {
  M5.begin();
  topBar.begin();
  topBar.setTitle("設定画面");
  
  btn.setCallback([](){
    Serial.println("実行ボタン押された");
  }, EVENT_TYPE::CLICK);
}

void loop() {
  topBar.update();
  btn.update();
  slider.update();
  wifiSwitch.update();
  
  M5.Canvas.clear();
  topBar.draw(M5.Canvas);
  
  M5.Canvas.drawString("音量:", 50, 60);
  slider.draw(M5.Canvas);
  
  M5.Canvas.drawString("WiFi:", 50, 110);
  wifiSwitch.draw(M5.Canvas);
  
  btn.draw(M5.Canvas);
  
  M5.Canvas.pushSprite(0, 0);
  delay(10);
}
```

---

## ❓ よくある質問

### Q: 全部のコンポーネントを使わないといけない？
**A**: いいえ、必要なものだけ使ってください。

### Q: 自分でコンポーネントを作れる？
**A**: はい！既存のコードを参考にして、同じパターンで作れます。

### Q: 色やサイズは変更できる？
**A**: はい、ほとんどのパラメータは作成時に指定できます。

### Q: Arduino IDEでも使える？
**A**: M5CoreS3用に設計されているので、M5Stack環境なら使えます。

---

## 🎯 まとめ

- **5種類の部品** が使える
- **同じパターン** で簡単に使える
- **独立している** ので好きなものだけ使える
- **初心者向け** の詳しいドキュメント完備

まずは `Button/` から始めてみましょう！
