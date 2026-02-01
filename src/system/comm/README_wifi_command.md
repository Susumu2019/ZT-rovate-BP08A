## AppWifi - WiFi/UDP接続画面

最終更新日: 2026年2月1日

### 🏷 役割
WiFi接続・UDP通信の状態表示や制御を行うアプリ画面です。

### 📁 ファイル
- `AppWifi.h` - クラス定義
- `AppWifi.cpp` - 実装

### 使い方
```cpp
appManager.registerApp(new AppWifi());
```


### UDP通信フォーマット・プロトコル例

- 送信（ロボット→PC）: `[AA55][roll][pitch][yaw][gx][gy][gz][temp]`（float*6+uint8, little endian）
- 受信（PC→ロボット）: `[AA55][angle0][angle1]...[angle7]`（各angleはu16リトルエンディアン, 0-180）

#### 送信例
| フィールド | サイズ | 内容 |
|---|---|---|
| roll/pitch/yaw | float*3 | 姿勢角度[deg] |
| gx/gy/gz | float*3 | ジャイロ[deg/s] |
| temp | uint8 | 温度 |

#### 受信例
| フィールド | サイズ | 内容 |
|---|---|---|
| angle0-7 | uint16*8 | サーボ角度[0-180] |

---

`AppWifi.cpp` の `handleTouch()` でWiFi/UDP制御ロジックを実装します。
