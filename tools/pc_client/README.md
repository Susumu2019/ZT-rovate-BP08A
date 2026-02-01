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


## UDP通信フォーマット・プロトコル

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
