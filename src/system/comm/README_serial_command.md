
# シリアル通信プロトコル詳細

## テキスト通信（JSON/1文字コマンド）

- 通信方式: UART2 (デフォルト 921600bps)
- 改行(\r, \n)でコマンド区切り
- 1文字コマンド、JSONコマンド両対応

### 送信例
```
p\n
{"cmd": "ping"}\n
{"cmd": "servo", "pos": [1500,1500,1500,1500,1500,1500,1500,1500]}\n
```
### 応答例
```
pong
{"resp": "pong", "millis": 1234567}
```

### コマンド一覧
- `p` : pong応答
- `{ "cmd": "ping" }` : pong応答(JSON)
- `{ "cmd": "servo", "pos": [...] }` : サーボ一括制御
- `{ "cmd": "offset", "off": [...] }` : サーボオフセット一括設定
- `{ "cmd": "set", "id": n, "val": v }` : 単一サーボ制御
- `{ "cmd": "reset" }` : サーボ全リセット
- `?` : コマンド説明表示

#### 注意事項
- 1コマンドごとに改行(\r, \n)が必要です。
- JSONコマンドはダブルクォートで記述してください。
- 不正なコマンドやJSONパースエラー時は応答しません。

---

## Hello World: Windowsターミナルでサーボを動かす

Windowsのターミナルから簡単にサーボを制御する手順です。

### 準備
1. デバイスをUSBで接続
2. デバイスマネージャで接続しているCOMポート番号を確認（例：COM5）
3. デバイスがテキスト通信モード（デフォルト）であることを確認

### 方法1: PowerShellを使用（シンプル）

PowerShellで次のコマンドを実行：

```powershell
# サーボID=0を中央位置(1500)から少し右に動かす(1700)へセット
$port = New-Object System.IO.Ports.SerialPort COM5,921600
$port.Open()
$port.WriteLine('{"cmd": "set", "id": 0, "val": 1700}')
Start-Sleep -Milliseconds 100
$response = $port.ReadLine()
Write-Host "応答: $response"
$port.Close()
```

### 方法2: 既存Pythonスクリプトを使用（推奨）

プロジェクトの `tools/pc_client/` には通信スクリプトが用意されています：

```bash
# ターミナルをプロジェクトルートで開く
cd c:\Users\zeate\Documents\PlatformIO\Projects\ZT-rovate-BP08A

# Pythonスクリプトを使用してサーボを制御
python tools/pc_client/serialcontrol.py COM5
```

スクリプト内で対話的にコマンドを入力：
```
> {"cmd": "set", "id": 0, "val": 1700}
```

### 動作確認の流れ

例：サーボ0を動かす手順

```
# 1. 接続確認
> p
pong

# 2. サーボ0を中央位置(1500)から右に動かす(1700)
> {"cmd": "set", "id": 0, "val": 1700}

# 3. すべてのサーボを中央位置にリセット
> {"cmd": "reset"}

# 4. 全サーボを同時に異なる位置に設定
> {"cmd": "servo", "pos": [1700,1500,1500,1500,1500,1500,1500,1500]}
```

**ポイント**
- サーボ値の範囲：500～2500（中央：1500）
- ID：0～7（8個のサーボ）
- 各コマンド実行後、応答がない場合は成功です（エラー時のみ出力）

---

## バイナリ通信

- デバイス設定: AppSetupで「Mode = BINARY」を選択し、再起動
- フレーム形式: `[AA55][VER][TYPE][SEQ2][LEN2][PAYLOAD][CRC16][7E]`
	- `AA55`: シンク (リトルエンディアン)
	- `VER=1`, `TYPE=0x01` (センサ送信) / `0x02` (コマンド受信)
	- `SEQ2`: シーケンス番号(2byte)
	- `LEN2`: ペイロード長(2byte)
	- `PAYLOAD`: データ本体
	- `CRC16-CCITT`: ヘッダ+ペイロードに対する
	- `7E`: ETX(終端, UART用)

### 受信例（センサデータ TYPE=0x01）
| フィールド | サイズ | 内容 |
|---|---|---|
| ax,ay,az | float*3 | 加速度 |
| gx,gy,gz | float*3 | ジャイロ |
| temp | uint8 | 温度 |
| pos[8] | uint16*8 | サーボ位置 |
| off[8] | uint16*8 | サーボオフセット |

### 送信例（コマンド TYPE=0x02）
- `cmd=0x01` (SET_SERVO): payload = [id:2][val:2]
- `cmd=0x02` (SET_ALL): payload = [pos[0:16]]
- `cmd=0x03` (RESET): payload = empty
- `cmd=0x04` (PING): payload = empty

#### 注意事項
- CRC16-CCITT(0x1021, init 0xFFFF)で検証
- 1フレームごとに7E終端
- 受信/送信ともにバイナリ形式

---

## Hello World: Windowsでバイナリ通信をテストする

バイナリモードでサーボを制御し、センサデータを受信する簡単な手順です。

### 準備
1. デバイスをUSBで接続（COMポート確認例：COM5）
2. デバイスをバイナリモードに設定（AppSetupで「Mode = BINARY」→ 再起動）
3. ボーレートはデフォルト921600bps

### 方法1: 既存スクリプト（最も簡単）

```bash
# プロジェクトルートで実行
cd c:\Users\zeate\Documents\PlatformIO\Projects\ZT-rovate-BP08A

# バイナリモードで起動（GUIで視覚的に確認）
python tools/pc_client/serialcontrol.py COM5 --binary --baud 921600
```

GUIが起動して、サーボスライダーで制御できます。IMU値もリアルタイム表示されます。

### 方法2: 簡単なPythonスクリプト（カスタム制御）

以下のPythonコードを `test_binary.py` として保存して実行：

```python
import serial
import struct
import time

def crc16_ccitt(data, poly=0x1021, init=0xFFFF):
    """CRC16-CCITT計算"""
    crc = init
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

def build_binary_packet(cmd, payload=b''):
    """バイナリパケットを構築"""
    sync = b'\xAA\x55'        # シンク
    ver = 0x01                # バージョン
    pkt_type = 0x02           # コマンド受信
    seq = 0x0001              # シーケンス
    length = len(payload)     # ペイロード長
    
    # ヘッダ構築
    header = sync + struct.pack('<BBHH', ver, pkt_type, seq, length) + payload
    
    # CRC計算
    crc = crc16_ccitt(header)
    crc_bytes = struct.pack('<H', crc)
    
    # 完全パケット
    packet = header + crc_bytes + b'\x7E'
    return packet

# シリアル接続
ser = serial.Serial('COM5', 921600, timeout=1)
time.sleep(0.5)

try:
    # [1] PING送信（cmd=0x04）
    print("=== PING送信 ===")
    ping_packet = build_binary_packet(0x04)
    ser.write(ping_packet)
    print(f"送信: {ping_packet.hex()}")
    
    # センサデータ受信待機（約100ms）
    time.sleep(0.1)
    if ser.in_waiting:
        response = ser.read(ser.in_waiting)
        print(f"受信: {response.hex()}")
        print(f"受信サイズ: {len(response)} bytes")
    
    # [2] サーボID=0を1700に設定（cmd=0x01）
    print("\n=== サーボID=0を1700に設定 ===")
    servo_payload = struct.pack('<HH', 0, 1700)  # id=0, val=1700
    servo_packet = build_binary_packet(0x01, servo_payload)
    ser.write(servo_packet)
    print(f"送信: {servo_packet.hex()}")
    
    time.sleep(0.1)
    if ser.in_waiting:
        response = ser.read(ser.in_waiting)
        print(f"受信: {response.hex()}")
    
    # [3] 全サーボを中央位置に設定（cmd=0x02）
    print("\n=== 全サーボを中央位置(1500)に設定 ===")
    all_servo_payload = struct.pack('<HHHHHHHH', 
                                    1500, 1500, 1500, 1500,
                                    1500, 1500, 1500, 1500)
    all_servo_packet = build_binary_packet(0x02, all_servo_payload)
    ser.write(all_servo_packet)
    print(f"送信: {all_servo_packet.hex()}")
    
    time.sleep(0.1)
    if ser.in_waiting:
        response = ser.read(ser.in_waiting)
        print(f"受信: {response.hex()}")

finally:
    ser.close()
    print("\n接続を閉じました")
```

実行方法：
```bash
# 必要なライブラリインストール
pip install pyserial

# スクリプト実行
python test_binary.py
```

### 方法3: PowerShellで直接送信

複雑になるため、方法1か方法2を推奨します。

**動作確認ポイント**
- PINGを送信するとセンサデータ（IMU + サーボ位置）が１フレーム帰ってきます
- サーボコマンドを送信後、次のセンサデータに反映されます
- バイナリパケットの16進表示で正確性を確認できます

**トラブルシューティング**
- 応答がない → ボーレート確認（デフォルト921600）
- パケット形式エラー → CRC計算確認
- センサデータが来ない → サーボデバイスの電源確認

---

最終更新: 2026-02-18
