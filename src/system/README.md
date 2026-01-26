## system（簡潔）

このフォルダはシステム基盤をまとめた場所です。簡素化方針により、`system` は初期化処理と共通ユーティリティ（グローバル変数など）を提供します。

主な内容
- `system.h` / `system.cpp` : 初期化処理と共通変数
- `touch/` : `TouchManager`（簡易化版）

使い方（要点）
1. `setup()` で `M5.begin()` を呼ぶ
2. 必要なグローバル初期化は `system.cpp` にまとめる
3. タッチ処理は `touch/README.md` を参照して `touchManager.update()` を使う

注意
- グローバル変数は便利ですが乱用しないこと（テストと保守性の観点から）。

参照
- 実装: `src/system/system.h`, `src/system/system.cpp`, `src/system/touch/`
