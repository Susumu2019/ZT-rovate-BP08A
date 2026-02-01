# rovate_Bipedal_TYPE-A

最終更新日: 2026年2月1日

<img src="lib/rovate_240_240.png" width="25%" alt="rovate logo">

**rovate** は **zeatec** が開発するロボットシリーズのブランド名です。
このプロジェクトは、 **rovate** の二足歩行ロボット **Bipedal Type-A** コントローラー用マイコンアプリケーションです。

M5Stack CoreS3をベースに、複数のアプリ（画面）で直感的にロボットを制御できるシステムになっています。


### 主な特徴
- タッチUIによる直感操作（ボタン・スライダー・スイッチ）
- シリアル通信（テキスト/バイナリ/JSON）・UDP通信対応
- PCクライアント（Python/pygame, UDP/シリアル）で外部制御・可視化
- IMU（加速度・ジャイロ）・サーボ・モーター制御
- 拡張性の高いアプリ構造（AppManager/テンプレート）

---

## 📑 目次

- [通信仕様・プロトコル](#通信仕様プロトコル)
- [このプロジェクトについて](#このプロジェクトについて)
- [ピンアサイン・外部接続・I2C回路構成について](#ピンアサイン外部接続i2c回路構成について)
- [クイックスタート（初心者向け）](#クイックスタート初心者向け)
- [プロジェクト構成（全ファイル一覧）](#プロジェクト構成全ファイル一覧)
- [学習の進め方（初心者向けロードマップ）](#学習の進め方初心者向けロードマップ)
- [重要なドキュメント（必読）](#重要なドキュメント必読)
- [よくある質問（FAQ）](#よくある質問faq)
- [カスタマイズのヒント](#カスタマイズのヒント)
- [トラブルシューティング](#トラブルシューティング)
- [さらに学ぶには](#さらに学ぶには)
- [ライセンス](#ライセンス)
- [作者](#作者)
- [バージョン情報](#バージョン情報)
- [クイックリファレンス](#クイックリファレンス)

---

---


<h2 id="通信仕様プロトコル">🔗 通信仕様・プロトコル</h2>

本プロジェクトは以下の通信方式に対応しています：

- **シリアル通信（テキスト/JSON/バイナリ）**
    - 詳細: [README_serial_command.md](README_serial_command.md)
- **UDP通信（PC⇔ロボット）**
    - 詳細: [src/App/AppWifi/README.md](src/App/AppWifi/README.md), [tools/pc_client/README.md](tools/pc_client/README.md)

通信コマンド例やフォーマットは各READMEに記載しています。


<h2 id="このプロジェクトについて">🎯 このプロジェクトについて</h2>

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





<h2 id="ピンアサイン外部接続i2c回路構成について">🛠️ ピンアサイン・外部接続・I2C回路構成について</h2>

<div align="center">
    <img src="datasheet/M5CoreS3-SE_ピンアサイン.jpg" alt="M5CoreS3-SE ピンアサイン" width="60%" style="max-width:400px; min-width:200px;">
</div>

### I2C回路構成（接続デバイス一覧）

本機は2系統のI2Cバスを持ち、各バスに以下のデバイスが接続されています（コード・スキャンアプリより）：

#### PORT.A（外部I2C, Wire）
- SDA: GPIO2
- SCL: GPIO1
- 主な接続デバイス例：
        - **PCA9685 サーボドライバ**（アドレス: 0x40）
        - **MPU6886 IMU**（アドレス: 0x68）
        - その他、外部I2Cデバイス（OLED, ADC, I/O拡張等）も接続可能

#### 内部I2C（Wire1）
- SDA: GPIO21
- SCL: GPIO22
- 主な接続デバイス例：
        - 内部拡張用（標準では特定デバイス未接続）

#### 代表的なI2Cデバイスとアドレス
| アドレス | デバイス名           |
|---------|---------------------|
| 0x68    | MPU6886 IMU         |
| 0x40    | PCA9685 サーボドライバ |
| 0x3C    | SSD1306 OLED        |
| 0x48    | ADS1115 ADC         |
| 0x27    | PCF8574 I/O         |
| 0x20    | MCP23017 I/O        |
| 0x76    | BME280 センサ       |
| 0x54/0x58| STAMPS3             |


### 主要ピンアサイン（M5Stack CoreS3）

- **30ピン（BAT）**: リチウムポリマーバッテリーが直接接続されています。外部から電源を供給する場合やバッテリー交換時は、必ず極性・電圧に注意してください。
    - BATピンはバッテリー電圧（3.7V系）がそのまま出力されます。
    - USB給電時は自動的に充電されます。
- **6ピン（WS2812）**: 外部接続のアドレサブルLED（WS2812, NeoPixel等）用データ出力ピンです。
    - デフォルトでは16個のWS2812 LEDに対応しています。
    - LEDの電源（5V/GND）は別途供給してください。
    - LEDの信号線は6ピン（GPIO6）に接続します。

#### WS2812（外部LED）の点灯色について

本体から外部WS2812（NeoPixel）LEDを制御しています。LEDの点灯色は主に以下のような意味を持ちます：

- **起動時**: 全LEDが黄色に点灯（システム起動中）
- **通常動作時**:
    - 先頭（0番目）: 赤色（基準マーカー）
    - 最後（15番目）: 青色（基準マーカー）
    - 中央付近（6,7,8番目）: 通信状態やバッテリー残量で色が変化
        - 通信OK時: 緑色で点滅
        - 通信NGや通常時: バッテリー残量に応じて色が変化
            - 30%未満: 赤
            - 30-40%: オレンジ
            - 40-60%: 黄
            - 60%以上: 白
    - その他のLED: バッテリー残量に応じて上記と同じ色で点灯

LEDの本数やピン番号は `src/main.cpp` の `#define WS2812_PIN 6` および `#define WS2812_COUNT 16` で変更可能です。

---

<h2 id="クイックスタート初心者向け">🚀 クイックスタート（初心者向け）</h2>

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



<h2 id="プロジェクト構成全ファイル一覧">📂 プロジェクト構成（全ファイル一覧）</h2>

```
.git/
.gitignore
.pio/
.pio.zip
.venv/
.vscode/
datasheet/
    └── M5CoreS3-SE_ピンアサイン.jpg
docs/
    └── TOUCH_ARCHITECTURE.md
include/
    ├── color_config.h
    ├── config.h
    └── README
lib/
    ├── MPU6886_AHRS/
    │   ├── examples/
    │   │   └── BasicOrientation/
    │   │       └── BasicOrientation.ino
    │   ├── imu_config.h
    │   ├── library.properties
    │   ├── LIBRARY_SUMMARY.md
    │   ├── MadgwickAHRS.cpp
    │   ├── MadgwickAHRS.h
    │   ├── MPU6886.cpp
    │   ├── MPU6886.h
    │   ├── MPU6886_AHRS.cpp
    │   ├── MPU6886_AHRS.h
    │   └── README.md
    ├── README
    ├── rovate_240_240.bmp
    └── rovate_240_240.png
output.txt
platformio.ini
README.md
src/
    ├── App/
    │   ├── App.h
    │   ├── AppAction/
    │   │   ├── AppAction.cpp
    │   │   ├── AppAction.h
    │   │   ├── AppActionData.cpp
    │   │   ├── README.md
    │   │   ├── ServoImuController.cpp
    │   │   └── ServoImuController.h
    │   ├── AppI2CScan/
    │   │   ├── AppI2CScan.cpp
    │   │   ├── AppI2CScan.h
    │   │   └── README.md
    │   ├── AppIMU/
    │   │   ├── AppIMU.cpp
    │   │   ├── AppIMU.h
    │   │   └── README.md
    │   ├── AppInfo/
    │   │   └── AppInfo.h
    │   ├── AppManager/
    │   │   ├── AppManager.cpp
    │   │   ├── AppManager.h
    │   │   └── README.md
    │   ├── AppManual/
    │   │   ├── AppManual.cpp
    │   │   ├── AppManual.h
    │   │   └── README.md
    │   ├── AppMotor/
    │   │   ├── AppMotor.cpp
    │   │   ├── AppMotor.h
    │   │   └── README.md
    │   ├── AppSetup/
    │   │   ├── AppSetup.cpp
    │   │   └── AppSetup.h
    │   ├── AppTemplete/
    │   │   ├── AppTemplate.cpp
    │   │   ├── AppTemplate.h
    │   │   └── README.md
    │   ├── AppWifi/
    │   │   ├── AppWifi.cpp
    │   │   └── AppWifi.h
    │   ├── HomeScreen/
    │   │   ├── HomeScreen.cpp
    │   │   ├── HomeScreen.h
    │   │   └── README.md
    │   └── README.md
    ├── main.cpp
    ├── README.md
    ├── system/
    │   ├── comm/
    │   │   ├── CommProtocol.cpp
    │   │   ├── CommProtocol.h
    │   │   ├── README_serial_command.md
    │   │   ├── README_wifi_command.md
    │   │   ├── SerialSender.cpp
    │   │   ├── SerialSender.h
    │   │   ├── UdpSender.cpp
    │   │   └── UdpSender.h
    │   ├── IMU_6886.cpp.bak
    │   ├── IMU_6886.h.bak
    │   ├── README.md
    │   ├── Settings.cpp
    │   ├── Settings.h
    │   ├── system.cpp
    │   ├── system.h
    │   └── touch/
    │       ├── README.md
    │       ├── TouchManager.cpp
    │       └── TouchManager.h
    ├── timer/
    │   ├── README.md
    │   ├── timer.cpp
    │   └── timer.h
    ├── UI/
    │   ├── Button/
    │   │   ├── Button.h
    │   │   ├── ButtonTouch.cpp
    │   │   ├── ButtonView.cpp
    │   │   └── README.md
    │   ├── Icon/
    │   │   ├── Icon.cpp
    │   │   ├── Icon.h
    │   │   └── README.md
    │   ├── README.md
    │   ├── SliderBar/
    │   │   ├── README.md
    │   │   ├── SliderBar.cpp
    │   │   └── SliderBar.h
    │   ├── Switch/
    │   │   ├── README.md
    │   │   ├── ToggleSwitch.cpp
    │   │   └── ToggleSwitch.h
    │   └── TopBar/
    │       ├── README.md
    │       ├── TopBar.cpp
    │       └── TopBar.h
test/
    └── README
tools/
    └── pc_client/
            ├── main.py
            ├── pose_memory.json
            ├── README.md
            ├── requirements.txt
            ├── RunUdpControl.bat
            ├── serialcontrol.bat
            ├── serialcontrol.py
            ├── serialcontrol_binary.bat
            ├── serialcontrol_binary_debug.bat
            ├── serialcontrol_text.bat
            ├── udpcontrol.py
            ├── 受信したバイナリデータをテキストに変換して表示.py
            └── 受信したバイナリデータを表示.py
```

💡 **各フォルダに詳しいREADME.mdがあります**：使い方や関数の説明を参照してください。

---


<h2 id="学習の進め方初心者向けロードマップ">📚 学習の進め方（初心者向けロードマップ）</h2>

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


<h2 id="重要なドキュメント必読">🎓 重要なドキュメント（必読）</h2>

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


<h2 id="よくある質問faq">💡 よくある質問（FAQ）</h2>

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


<h2 id="カスタマイズのヒント">🔧 カスタマイズのヒント</h2>

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


<h2 id="トラブルシューティング">🐛 トラブルシューティング</h2>

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


<h2 id="さらに学ぶには">📖 さらに学ぶには</h2>

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


<h2 id="ライセンス">📝 ライセンス</h2>

このプロジェクトは MIT License です。
自由に使用・改変・配布できます。

---


<h2 id="作者">👨‍💻 作者</h2>

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


<h2 id="バージョン情報">📌 バージョン情報</h2>

- **Version**: 1.0
- **Last Updated**: 2025-11-10
- **Platform**: PlatformIO
- **Board**: M5Stack CoreS3
- **Framework**: Arduino

---


<h2 id="クイックリファレンス">⚡ クイックリファレンス</h2>

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
