# PCクライアント (シリアル制御)

最終更新日: 2026年2月1日

M5Stack CoreS3 のファームウェアと通信する pygame クライアント。
テキストJSONモードとバイナリプロトコルの両方に対応。

## 機能
- IMU可視化: 加速度から算出したロール/ピッチで立方体＋XYZ軸を回転表示
- 最新のシーケンス番号 (`seq`) を表示
- 8サーボをスライダでリアルタイム操作（`set` コマンド送信）
- スクリプトモード: 簡易コマンドを複数行入力しループ実行
- **バイナリプロトコル対応**: テキストモードより高速・効率的な通信

## 前提
- Python 3.9+
- 依存インストール:
  ```bash
  pip install -r requirements.txt
  ```

## 実行

### テキストJSON モード（デフォルト）
```bash
python main.py --port COM3 --baud 921600
```

### バイナリモード
```bash
python main.py --port COM3 --baud 921600 --binary
```

オプション:
- `--port`: シリアルポート (例: COM3, /dev/ttyUSB0)。省略時に自動検出
- `--baud`: ボーレート (デフォルト 921600)
- `--binary`: バイナリプロトコルを使用 (デフォルトはテキストJSON)
- `--fullscreen`: フルスクリーン起動
- `--width` / `--height`: ウィンドウサイズ (デフォルト 1200x700)

## プロトコル

### テキストJSON モード
- **デバイス設定**: AppSetupで「Mode = TEXT」を選択し、ファームを再起動
- **受信** (JSON 1行): `seq`, `imu`(ax,ay,az,gx,gy,gz,temp), `pos`, `off` を含むJSON
- **送信** (JSON + 改行):
  - `{"cmd":"set","id":0,"val":1500}`
  - `{"cmd":"servo","pos":[1500,...]}`
  - `{"cmd":"offset","off":[0,...]}`
  - `{"cmd":"reset"}`
  - `{"cmd":"ping"}`

### バイナリモード
- **デバイス設定**: AppSetupで「Mode = BINARY」を選択し、ファームを再起動
- **フレーム形式**: `[AA55][VER][TYPE][SEQ2][LEN2][PAYLOAD][CRC16][7E]`
  - `AA55`: シンク (リトルエンディアン)
  - `VER=1`, `TYPE=0x01` (センサ送信) / `0x02` (コマンド受信)
  - `CRC16-CCITT`: ペイロードに対する
  - `7E`: ETX
- **受信**: センサデータ (TYPE=0x01): imu(28B) + pos(16B) + off(16B)
- **送信**: コマンド (TYPE=0x02)
  - `cmd=0x01` (SET_SERVO): payload = [id:2][val:2]
  - `cmd=0x02` (SET_ALL): payload = [pos[0:16]]
  - `cmd=0x03` (RESET): payload = empty
  - `cmd=0x04` (PING): payload = empty

## スクリプトモード（ループ）
テキストボックスに1行1コマンドで記述し、`Run Script` で開始、`Stop Script` で停止。
サポートコマンド:
- `set <id> <val>`: サーボ id (0-7) に val (500-2500) を設定
- `wait <sec>`: sec 秒待機（少数可）
- `ping`: ping送信
- `reset`: 全サーボリセット
- `seq`: ローカルでseqを表示（送信はしない）
`#` で始まる行はコメントとして無視。

例:
```
set 0 1200
wait 0.5
set 0 1800
wait 0.5
ping
```

## 操作
- スライダをドラッグしてサーボ値を送信（一定間隔で送信）
- `Run Script` / `Stop Script` ボタン
- `Ping` ボタンで疎通確認



---

## 各ファイルの役割・使い方

### main.py
- **役割**: PCクライアントのメインGUIアプリ（pygameベース）。シリアル/バイナリ通信でロボットを制御・可視化。
- **使い方**: `python main.py --port COM8 --baud 921600` など。GUIでサーボ操作・IMU可視化・スクリプト実行。
- **使用例**: サーボスライダーを動かすとリアルタイムでロボットが動作。

### serialcontrol.py
- **役割**: コマンドラインからシリアル通信でロボット制御・状態取得。バイナリ/テキスト両対応。
- **使い方**: `python serialcontrol.py --port COM8 --baud 921600 --binary` など。コマンド送信や受信データの表示。
- **使用例**: `--binary`でバイナリ通信、`--port`でポート指定。

### udpcontrol.py
- **役割**: UDP通信でロボットを制御・IMU値を受信するTkinter GUIアプリ。
- **使い方**: `python udpcontrol.py` で起動。IP/ポート指定、サーボ操作、IMU値の3D表示。
- **使用例**: サーボスライダーで角度を送信、IMU値が表や3Dで可視化。

### requirements.txt
- **役割**: Python依存パッケージリスト。
- **使い方**: `pip install -r requirements.txt` で必要なライブラリを一括インストール。

### pose_memory.json
- **役割**: サーボのポーズ（角度セット）を記録・再生するためのJSONデータ。
- **使い方**: main.py等から自動的に読み書き。

### udpcontrol.bat
- **役割**: 仮想環境のPythonでudpcontrol.pyを実行するWindowsバッチ。
- **使い方**: ダブルクリックでUDPコントローラGUIを起動。

### serialcontrol.bat / serialcontrol_text.bat
- **役割**: serialcontrol.pyをテキスト通信モードで起動。
- **使い方**: ダブルクリックでCOM8, 921600bpsで起動。

### serialcontrol_binary.bat / serialcontrol_binary_debug.bat
- **役割**: serialcontrol.pyをバイナリ通信モードで起動（debugは詳細出力）。
- **使い方**: ダブルクリックでCOM8, 921600bps, バイナリ通信で起動。

### 受信したバイナリデータをテキストに変換して表示.py
- **役割**: シリアル受信データをテキスト変換して表示。
- **使い方**: `python 受信したバイナリデータをテキストに変換して表示.py`。改行区切りでテキスト表示。

### 受信したバイナリデータを表示.py
- **役割**: シリアル受信データをバイナリ（16進）で表示。
- **使い方**: `python 受信したバイナリデータを表示.py`。AA55ヘッダでパケット分割し16進表示。

---

### 送信（PC→ロボット）
- サーボ角度一括送信: `[AA55][angle0][angle1]...[angle7]`（各angleはu16リトルエンディアン, 0-180）
  - 例: `AA 55 5A 00 5A 00 ...`（8ch分, 90度=0x5A）

### 受信（ロボット→PC）
- IMUデータ: `[AA55][roll][pitch][yaw][gx][gy][gz][temp]`（float*6+uint8, little endian）
  - 例: `AA 55 <float*6> <uint8>`
  - roll/pitch/yaw: degree, gx/gy/gz: deg/s, temp: 温度

### 注意事項
- UDPはコネクションレス。パケットロスに注意。
- 送信先IP/PORTはROBOT_IP/ROBOT_PORTで指定。
- 受信側はLOCAL_PORTで待ち受け。

---
