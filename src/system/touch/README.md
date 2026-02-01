## touch - タッチ入力マネージャ

最終更新日: 2026年2月1日

### 🎯 このクラスの目的

タッチパネルの入力を **1か所で管理** するためのクラスです。

**なぜ必要？**:
- 各コンポーネント（ボタン、スライダーなど）が個別にタッチを読むと、座標がずれたり二重反応したりする
- TouchManagerが **一元的に** タッチ情報を提供することで、一貫性が保たれる

💡 **シングルトン**: プログラム全体で1つだけ存在します。

---
# touch（簡素化版）

このファイルはタッチ入力の使い方を短くまとめたものです。

設計方針（簡素化）
--
- `TouchManager` は最小限の情報のみを提供します：座標、現在タッチ中か、押下/離上の簡易エッジ。
- 複雑なジェスチャやマルチタッチの管理は行わず、UI 側で必要な判定を行う想定です。

基本ルール
--
1. `loop()` 内で毎フレーム `touchManager.update()` を呼んでください。
2. タッチ状態の参照には `getX()/getY()/isTouched()/wasPressed()/wasReleased()` を使います。

サンプル
--
```cpp
touchManager.update();
if (touchManager.wasPressed()) onPress(touchManager.getX(), touchManager.getY());
if (touchManager.isTouched()) onMove(touchManager.getX(), touchManager.getY());
if (touchManager.wasReleased()) onRelease(touchManager.getX(), touchManager.getY());
```

注意
--
- 長押しや連続イベントなどは UI 側（ボタンやスライダー）で実装してください。
- マルチタッチは本テンプレートでは想定していません。

参照
--
- 実装: `src/system/touch/TouchManager.h` / `src/system/touch/TouchManager.cpp`
