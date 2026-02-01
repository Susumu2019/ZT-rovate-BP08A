# rovate_Bipedal_TYPE-A

最終更新日: 2026年2月1日

<img src="lib/rovate_240_240.png" width="25%" alt="rovate logo">

**rovate** は **zeatec** が開発するロボットシリーズのブランド名です。
このプロジェクトは、 **rovate** の二足歩行ロボット **Bipedal Type-A** コントローラー用マイコンアプリケーションです。
M5Stack CoreS3をベースに、複数のアプリ（画面）で直感的にロボットを制御できるシステムになっています。

---

## 🎯 このプロジェクトについて

### 何ができるの？

このプロジェクトは、rovate Bipedal Type-Aの制御に以下のような機能を備えています：

- 📱 **複数の制御画面** でロボットの各機能を操作
- 🖱️ **タッチ操作** に対応したUI部品（ボタン、スライダー、スイッチ）
- 🏠 **ホーム画面** から各種操作モードを選択
- 🔄 **モード切り替え** が簡単
- ⏱️ **タイマー機能** で定期的なセンサー更新や制御処理
- 🔌 **I2C通信** で外部デバイス（モーター、センサーなど）と接続

### 対象ハードウェア

- **M5Stack CoreS3** (SE含む) - rovate Bipedal Type-A コントローラー
- タッチスクリーン対応 - ロボット制御用UI
- ESP32ベース - 高速処理対応
- **I2C通信機能** - モーター制御、センサー接続用

---

## 🚀 クイックスタート（初心者向け）

### 必要なもの

1. **M5Stack CoreS3** 本体
2. **パソコン**（Windows/Mac/Linux）
3. **USBケーブル**（Type-C）
4. **開発環境**：
   - [VS Code](https://code.visualstudio.com/)
   - [PlatformIO](https://platformio.org/)（VS Codeの拡張機能）

---

### ステップ1: 開発環境のセットアップ

#### 1-1. VS Codeをインストール

[https://code.visualstudio.com/](https://code.visualstudio.com/) からダウンロードしてインストール

#### 1-2. PlatformIOをインストール

1. VS Codeを開く
2. 左側の「拡張機能」アイコンをクリック（四角が4つのアイコン）
3. 検索窓に「PlatformIO」と入力
4. 「PlatformIO IDE」をインストール
5. VS Codeを再起動

💡 **初回は時間がかかります**（10-20分）：必要なツールを自動でダウンロードします。

---

### ステップ2: プロジェクトを開く

1. VS Codeで「ファイル」→「フォルダーを開く」
2. このプロジェクトのフォルダ（`rovate_Bipedal_TYPE-A`）を選択
3. 左下に「PlatformIO」のアイコンが表示されればOK

---

### ステップ3: ビルドして実行

#### 3-1. M5Stack CoreS3を接続

USBケーブルでパソコンとM5Stackを接続します。

#### 3-2. ビルド（コンパイル）

1. 画面下部の「✓」アイコンをクリック（Build）
2. 初回は時間がかかります（ライブラリをダウンロード）
3. 「SUCCESS」と表示されればOK

#### 3-3. アップロード（書き込み）

1. 画面下部の「→」アイコンをクリック（Upload）
2. M5Stackに書き込まれます
3. 自動的にプログラムが起動します

#### 3-4. 動作確認

- ホーム画面にアプリアイコンが表示される
- アイコンをタッチするとアプリが起動
- 画面下部のボタンBでホーム画面に戻る

🎉 **成功！** プログラムが動いたら、次はカスタマイズしてみましょう。

---

## 📂 プロジェクト構成

```
M5CoreS3SE_Template-main/
├── platformio.ini          # PlatformIO設定ファイル
├── README.md               # このファイル
├── include/                # ヘッダファイル（グローバル設定）
│   ├── config.h           # 画面サイズなどの設定
│   ├── color_config.h     # 色の定義
│   └── README
├── src/                    # ソースコード（ここが重要！）
│   ├── main.cpp           # メインプログラム（エントリポイント）
│   ├── README.md          # ソースコード全体の説明
│   ├── App/               # アプリケーション層
│   │   ├── App.h          # アプリの基底クラス
│   │   ├── README.md      # アプリ関連の説明
│   │   ├── AppManager/    # アプリ管理システム
│   │   │   ├── AppManager.h
│   │   │   ├── AppManager.cpp
│   │   │   └── README.md
│   │   ├── AppInfo/       # 情報表示アプリ
│   │   │   ├── AppInfo.h
│   │   │   ├── AppInfo.cpp
│   │   │   └── README.md
│   │   ├── AppLock/       # ロック画面アプリ
│   │   │   ├── AppLock.h
│   │   │   ├── AppLock.cpp
│   │   │   └── README.md
│   │   ├── AppMotor/      # モーター制御アプリ
│   │   │   ├── AppMotor.h
│   │   │   ├── AppMotor.cpp
│   │   │   └── README.md
│   │   ├── AppTemplete/   # 新規アプリのテンプレート
│   │   │   ├── AppTemplate.h
│   │   │   ├── AppTemplate.cpp
│   │   │   └── README.md
│   │   └── HomeScreen/    # ホーム画面
│   │       ├── HomeScreen.h
│   │       ├── HomeScreen.cpp
│   │       └── README.md
│   ├── UI/                # ユーザーインターフェース部品
│   │   ├── README.md
│   │   ├── Button/        # ボタン
│   │   │   ├── Button.h
│   │   │   ├── ButtonView.cpp
│   │   │   ├── ButtonTouch.cpp
│   │   │   └── README.md
│   │   ├── Icon/          # アイコン
│   │   │   ├── Icon.h
│   │   │   ├── Icon.cpp
│   │   │   └── README.md
│   │   ├── SliderBar/     # スライダー
│   │   │   ├── SliderBar.h
│   │   │   ├── SliderBar.cpp
│   │   │   └── README.md
│   │   ├── Switch/        # トグルスイッチ
│   │   │   ├── ToggleSwitch.h
│   │   │   ├── ToggleSwitch.cpp
│   │   │   └── README.md
│   │   └── TopBar/        # 画面上部バー
│   │       ├── TopBar.h
│   │       ├── TopBar.cpp
│   │       └── README.md
│   ├── system/            # システム基盤
│   │   ├── README.md
│   │   ├── system.h       # システム全体の設定
│   │   ├── system.cpp
│   │   └── touch/         # タッチ入力管理
│   │       ├── README.md
│   │       ├── TouchManager.h
│   │       └── TouchManager.cpp
│   ├── timer/             # ハードウェアタイマー
│   │   ├── README.md
│   │   ├── timer.h
│   │   └── timer.cpp
├── lib/                    # ライブラリフォルダ
│   └── README
├── test/                   # テストフォルダ
│   └── README
└── docs/                   # ドキュメント
    └── TOUCH_ARCHITECTURE.md
```

💡 **各フォルダに詳しいREADME.mdがあります**：使い方や関数の説明を参照してください。

---

## 📚 学習の進め方（初心者向けロードマップ）

### レベル1: プロジェクトを動かす（30分）

1. ✅ 開発環境をセットアップ
2. ✅ プロジェクトをビルド＆アップロード
3. ✅ 動作を確認

### レベル2: コードを読んで理解する（1-2時間）

1. 📖 [`src/main.cpp`](src/main.cpp) を読む（プログラムの入口）
2. 📖 [`src/README.md`](src/README.md) を読む（全体構造を理解）
3. 📖 [`src/UI/Button/README.md`](src/UI/Button/README.md) を読む（ボタンの使い方）

### レベル3: 簡単な変更をしてみる（30分-1時間）

1. 🔧 ボタンの色を変えてみる
2. 🔧 表示する文字を変えてみる
3. 🔧 新しいボタンを追加してみる

### レベル4: 新しいアプリを作る（2-3時間）

1. 🎨 [`src/App/AppTemplete/README.md`](src/App/AppTemplete/README.md) を参考に
2. 🎨 テンプレートをコピーして新しいアプリを作成
3. 🎨 ボタンやスライダーを配置
4. 🎨 AppManagerに登録して動かす

---

## 🎓 重要なドキュメント（必読）

### 初心者向け
- 📘 [src/README.md](src/README.md) - ファイル構成の基本
- 📘 [src/UI/Button/README.md](src/UI/Button/README.md) - ボタンの使い方
- 📘 [src/App/README.md](src/App/README.md) - アプリの作り方

### 中級者向け
- 📗 [src/App/AppManager/README.md](src/App/AppManager/README.md) - アプリ管理システム
- 📗 [src/system/touch/README.md](src/system/touch/README.md) - タッチ入力の仕組み
- 📗 [src/timer/README.md](src/timer/README.md) - タイマーと割り込み

### 上級者向け
- 📕 [docs/TOUCH_ARCHITECTURE.md](docs/TOUCH_ARCHITECTURE.md) - タッチアーキテクチャ

---

## 💡 よくある質問（FAQ）

### Q1: プログラムが書き込めない

**A**: 以下を確認してください：
1. USBケーブルがデータ転送対応か（充電専用ケーブルはNG）
2. M5Stackの電源が入っているか
3. 正しいCOMポートが選択されているか（PlatformIOが自動選択）
4. 他のプログラムがポートを使用していないか

### Q2: コンパイルエラーが出る

**A**: 以下を試してください：
1. PlatformIOのライブラリを更新：`Ctrl+Shift+P` → `PlatformIO: Update All`
2. `.pio` フォルダを削除して再ビルド
3. エラーメッセージをよく読む（どのファイルの何行目か確認）

### Q3: タッチが反応しない

**A**: 以下を確認：
1. `touchManager.update()` を `loop()` 内で呼んでいるか
2. ボタンの `update()` を呼んでいるか
3. タッチ座標がボタンの範囲内か（デバッグ出力で確認）

### Q4: 画面が真っ暗

**A**: 以下を確認：
1. `M5.begin()` または `CoreS3.begin()` を呼んでいるか
2. `canvas.pushSprite()` を呼んでいるか
3. 明るさ設定が最低になっていないか

### Q5: 新しいアプリを追加したい

**A**: [`src/App/AppTemplete/README.md`](src/App/AppTemplete/README.md) を参照してください。
コピー→名前変更→実装の3ステップで作れます。

---

## 🔧 カスタマイズのヒント

### 画面サイズを変更

`include/config.h` を編集：
```cpp
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
```

### 色を変更

`include/color_config.h` を編集：
```cpp
#define DARKCYAN 0x03EF
```

### 新しいUIコンポーネントを追加

1. `src/UI/Button/` をコピー
2. 名前を変更（例: `MyWidget`）
3. 必要な機能を実装
4. 他のアプリから使う

---

## 🐛 トラブルシューティング

### コンパイルが遅い

**原因**: 初回ビルド時はライブラリをダウンロードするため時間がかかります。

**解決**: 2回目以降は速くなります。待ちましょう。

### メモリ不足エラー

**原因**: プログラムが大きすぎる、またはグローバル変数が多すぎる。

**解決**:
1. 不要なライブラリを削除
2. グローバル変数を減らす
3. `String` より `char[]` を使う

### 画面がちらつく

**原因**: 描画が遅い、または `canvas.clear()` の位置が間違っている。

**解決**:
1. 描画処理を最適化
2. `canvas.clear()` → 描画 → `pushSprite()` の順番を守る
3. 不要な再描画を減らす

---

## 📖 さらに学ぶには

### 公式ドキュメント
- [M5Stack公式サイト](https://docs.m5stack.com/)
- [PlatformIO公式ドキュメント](https://docs.platformio.org/)
- [Arduino言語リファレンス](https://www.arduino.cc/reference/en/)

### コミュニティ
- [M5Stack公式フォーラム](https://community.m5stack.com/)
- [Arduino日本語フォーラム](https://forum.arduino.cc/)

### サンプルコード
- [M5Stack Examples](https://github.com/m5stack/M5Stack/tree/master/examples)
- このプロジェクトの各フォルダの README.md

---

## 🤝 貢献

バグ報告や機能追加の提案は Issues へ。
プルリクエストも歓迎します！

---

## 📝 ライセンス

このプロジェクトは MIT License です。
自由に使用・改変・配布できます。

---

## 👨‍💻 作者

**ZEATEC co.,ltd. by Susumu.Hirai**

---

## 🎉 最後に

このテンプレートを使って、楽しいマイコンプロジェクトを作ってください！

困ったことがあれば：
1. 各フォルダの README.md を読む
2. サンプルコードを試す
3. エラーメッセージを検索する
4. Issues で質問する

**Happy Coding! 🚀**

---

## 📌 バージョン情報

- **Version**: 1.0
- **Last Updated**: 2025-11-10
- **Platform**: PlatformIO
- **Board**: M5Stack CoreS3
- **Framework**: Arduino

---

## ⚡ クイックリファレンス

### よく使うPlatformIOコマンド

```bash
# ビルド
pio run

# アップロード
pio run --target upload

# シリアルモニタ
pio device monitor

# クリーン（キャッシュ削除）
pio run --target clean

# 全てのライブラリを更新
pio pkg update
```

### VS Codeショートカット

- `Ctrl + Shift + B` : ビルド
- `Ctrl + Shift + P` : コマンドパレット
- `Ctrl + /` : コメントトグル
- `Ctrl + K, Ctrl + C` : コメント化
- `Ctrl + K, Ctrl + U` : コメント解除

---

**プロジェクトを楽しんでください！** 🎊
