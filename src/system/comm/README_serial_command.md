
# シリアル通信プロトコル詳細

最初にシリアル通信で制御を始める場合は、以下の初期設定セクションを参照してください。

---

## 初期設定（セットアップ）

### 概要

M5Stack CoreS3はUSB Type-C でPCに接続され、シリアル通信（RS-232）でロボットのサーボ・センサーを制御・受信できます。

### ⚠️ 重要：config.h の作成

**シリアル通信設定（ボーレート等）は `include/config.h` に保存されています。このファイルは .gitignore で除外されているため、個別に作成する必要があります：**

1. [include/config.h.template](../../../../include/config.h.template) を `include/config.h` にコピー：
   ```bash
   cd include
   copy config.h.template config.h
   ```

### ステップ1: ボーレート確認

デフォルトのボーレートは **921600 bps** です。変更が必要な場合は `config.h` を編集：

```cpp
// シリアル送信設定
static constexpr uint32_t SERIAL_BAUD = 921600;  // ボーレート（bps）
```

### ステップ1.5: シリアルポート詳細設定（PC側）

このファームのUART設定は **8N1 / フロー制御なし** です。PC側ツールも同じ値にしてください。

| 項目 | 設定値 |
|---|---|
| ボーレート | 921600 |
| データビット | 8 |
| パリティ | None |
| ストップビット | 1 |
| フロー制御 | None (XON/XOFF, RTS/CTS, DSR/DTR すべてOFF) |
| 改行 | 1コマンドごとに `LF(\n)` または `CRLF(\r\n)` |

PowerShellの`SerialPort`で明示する場合：

```powershell
$port = New-Object System.IO.Ports.SerialPort COM5,921600,None,8,one
$port.Handshake = [System.IO.Ports.Handshake]::None
$port.NewLine = "`n"
$port.ReadTimeout = 500
$port.WriteTimeout = 500
```

### ステップ2: COMポート確認

**Windows：デバイスマネージャーで確認**
1. Windowsスタートメニューで「デバイスマネージャ」を検索
2. 「ポート（COM と LPT）」を展開
3. M5Stack CoreS3を接続した状態で「USB Serial Device (COMx)」を確認
   - 例：COM5、COM3 など

**Windows：PowerShellで確認**
```powershell
Get-WmiObject Win32_SerialPort | Select-Object Description,DeviceID,Name
```

**macOS/Linux：ターミナルで確認**
```bash
# macOS
ls /dev/tty.*

# Linux
ls /dev/ttyUSB* 或いは ls /dev/ttyACM*
```

### ステップ3: テキスト/バイナリモード選択

2つの通信モードがあります。初期は **テキストモード（JSON）** がおすすめです：

| モード | 特徴 | 推奨用途 |
|--------|------|---------|
| **テキスト（JSON）** | 可読性高、デバッグ容易 | 初期開発・動作確認 |
| **バイナリ** | 高速、通信効率良好 | 本格運用・リアルタイム制御 |

**モード切り替え：**
1. ロボットの HomeScreen → AppSetup → Mode
2. TEXT or BINARY を選択
3. 再起動

### ステップ4: 簡単な接続テスト

PCとロボットがシリアルで通信可能か確認します：

**PowerShellでの簡単テスト（Windows）：**
```powershell
$port = New-Object System.IO.Ports.SerialPort COM5,921600
$port.Handshake = [System.IO.Ports.Handshake]::None
$port.NewLine = "`n"
$port.ReadTimeout = 500
$port.Open()
$port.WriteLine('p')
Start-Sleep -Milliseconds 200
Write-Host "応答: " $port.ReadExisting()
$port.Close()
```

期待される出力：
```
応答:  ping ok.
```

**Python スクリプトでの対話的テスト（推奨）：**
```bash
# プロジェクトルートに移動
cd c:\Users\zeate\Documents\PlatformIO\Projects\ZT-rovate-BP08A

# Pythonスクリプトを実行（仮想環境有効化後）
python tools/pc_client/serialcontrol.py --port COM5 --baud 921600
```

対話形式でコマンドを入力できます：
```
COM5 接続中...
> p
ping ok.
> {"cmd": "ping"}
{"resp": "ping", "millis": 123456}
```

---

- 通信方式: UART2 (デフォルト 921600bps)
- 改行(\r, \n)でコマンド区切り
- 1文字コマンド、JSONコマンド両対応

### 送信例
```
p\n
{"cmd": "ping"}\n
{"cmd": "servo", "pos": [90,90,90,90,90,90,90,90]}\n
```
### 応答例
```
ping ok.
{"resp": "ping", "millis": 1234567}
```

### コマンド一覧
- `p` : `ping ok.` 応答
- `{ "cmd": "ping" }` : `{"resp":"ping","millis":...}` 応答(JSON)
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
# サーボID=0を中央位置(90)から少し右に動かす(120)へセット
$port = New-Object System.IO.Ports.SerialPort COM5,921600
$port.Handshake = [System.IO.Ports.Handshake]::None
$port.NewLine = "`n"
$port.Open()
$port.WriteLine('{"cmd": "set", "id": 0, "val": 120}')
Start-Sleep -Milliseconds 100
# setコマンドは通常、成功時に応答を返しません
$port.WriteLine('p')
Start-Sleep -Milliseconds 200
Write-Host "応答: $($port.ReadExisting())"
$port.Close()
```

### 方法2: 既存Pythonスクリプトを使用（推奨）

プロジェクトの `tools/pc_client/` には通信スクリプトが用意されています：

```bash
# ターミナルをプロジェクトルートで開く
cd c:\Users\zeate\Documents\PlatformIO\Projects\ZT-rovate-BP08A

# Pythonスクリプトを使用してサーボを制御
python tools/pc_client/serialcontrol.py --port COM5 --baud 921600
```

スクリプト内で対話的にコマンドを入力：
```
> {"cmd": "set", "id": 0, "val": 120}
```

### 動作確認の流れ

例：サーボ0を動かす手順

```
# 1. 接続確認
> p
ping ok.

# 2. サーボ0を中央位置(90)から右に動かす(120)
> {"cmd": "set", "id": 0, "val": 120}

# 3. すべてのサーボを中央位置にリセット
> {"cmd": "reset"}

# 4. 全サーボを同時に異なる位置に設定
> {"cmd": "servo", "pos": [120,90,90,90,90,90,90,90]}
```

**ポイント**
- サーボ値の範囲：0～180（中央：90）
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
- `cmd=0x01` (SET_SERVO): payload = [0x01][id:1][val:2]
- `cmd=0x02` (SET_ALL): payload = [0x02][pos0:2][pos1:2]...[pos7:2]
- `cmd=0x03` (RESET): payload = [0x03]
- `cmd=0x04` (PING): payload = [0x04]

#### 注意事項
- CRC16-CCITT(0x1021, init 0xFFFF)で検証（`VER` ～ `PAYLOAD` を対象、`AA55`は対象外）
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
python tools/pc_client/serialcontrol.py --port COM5 --binary --baud 921600
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

def build_binary_packet(cmd, payload=b'', seq=1):
    """バイナリパケットを構築"""
    sync = b'\xAA\x55'        # シンク
    ver = 0x01                # バージョン
    pkt_type = 0x02           # コマンド受信
    full_payload = bytes([cmd]) + payload
    length = len(full_payload) # ペイロード長
    
    # ヘッダ + ペイロード
    packet_wo_crc = sync + struct.pack('<BBHH', ver, pkt_type, seq, length) + full_payload
    
    # CRC計算（VER～PAYLOAD。SYNC(AA55)は除外）
    crc = crc16_ccitt(packet_wo_crc[2:])
    crc_bytes = struct.pack('<H', crc)
    
    # 完全パケット
    packet = packet_wo_crc + crc_bytes + b'\x7E'
    return packet

# シリアル接続
ser = serial.Serial('COM5', 921600, timeout=0.5)
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
    
    # [2] サーボID=0を120度に設定（cmd=0x01）
    print("\n=== サーボID=0を120度に設定 ===")
    servo_payload = struct.pack('<BH', 0, 120)  # id=0(1byte), val=120(2byte)
    servo_packet = build_binary_packet(0x01, servo_payload)
    ser.write(servo_packet)
    print(f"送信: {servo_packet.hex()}")
    
    time.sleep(0.1)
    if ser.in_waiting:
        response = ser.read(ser.in_waiting)
        print(f"受信: {response.hex()}")
    
    # [3] 全サーボを中央位置(90度)に設定（cmd=0x02）
    print("\n=== 全サーボを中央位置(90度)に設定 ===")
    all_servo_payload = struct.pack('<HHHHHHHH', 
                                    90, 90, 90, 90,
                                    90, 90, 90, 90)
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

## バイナリ通信による複数サーボ同時制御

複数のサーボを1パケットで同時に制御する方法を説明します。

### パケット構造の詳細（SET_ALL, cmd=0x02）

8本のサーボを1フレームで全部書き換えます。

```
[AA][55][01][02][seq_lo][seq_hi][11][00][02][pos0_lo][pos0_hi][pos1_lo][pos1_hi]...[pos7_lo][pos7_hi][crc_lo][crc_hi][7E]
 ↑   ↑   ↑   ↑   ←SEQ (LE)→   ←LEN=17 (LE)→  ↑   ←────── pos[0..7] 各2byte LE ──────→  ←CRC (LE)→   ↑
SYNC      VER TYPE                              cmd=0x02                                                ETX
```

| Offset | サイズ | 内容 |
|--------|--------|------|
| 0–1    | 2      | SYNC: `AA 55` |
| 2      | 1      | VER: `01` |
| 3      | 1      | TYPE: `02`（PCからデバイスへのコマンド） |
| 4–5    | 2      | SEQ（リトルエンディアン） |
| 6–7    | 2      | LEN=17（リトルエンディアン） |
| 8      | 1      | CMD: `02`（SET_ALL_SERVOS） |
| 9–24   | 16     | pos[0]〜pos[7]（各 uint16 LE） |
| 25–26  | 2      | CRC16-CCITT（byte[2]〜byte[24]対象、LE） |
| 27     | 1      | ETX: `7E` |

**合計 28 bytes**

### Pythonによる複数サーボ制御例

以下のコードを `multi_servo_control.py` として保存して実行できます。

```python
import serial
import struct
import time

# ── ヘルパー関数 ──────────────────────────────────────────

def crc16_ccitt(data, poly=0x1021, init=0xFFFF):
    """CRC16-CCITT計算（VER〜PAYLOAD を対象）"""
    crc = init
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ poly) if (crc & 0x8000) else (crc << 1)
            crc &= 0xFFFF
    return crc

def build_set_all(positions, seq=1):
    """
    8本サーボを同時に指定角度に設定するパケットを構築する。
    positions: サーボ角度のリスト（長さ8、0〜180度、中立=90）
    """
    assert len(positions) == 8, "positions は8要素必要"
    positions = [max(0, min(180, p)) for p in positions]  # クランプ

    # ヘッダ: SYNC(2) + VER(1) + TYPE(1) + SEQ(2) + LEN(2) = 8bytes
    header = b'\xAA\x55' + struct.pack('<BBHH', 0x01, 0x02, seq, 17)
    # ペイロード: CMD(1) + pos[0..7] (各uint16 LE = 2bytes) = 17bytes
    payload = struct.pack('<B' + 'H'*8, 0x02, *positions)

    crc = crc16_ccitt(header[2:] + payload)   # VER〜PAYLOAD を対象
    return header + payload + struct.pack('<H', crc) + b'\x7E'

def build_set_single(servo_id, angle, seq=1):
    """
    1本のサーボを指定角度に設定するパケットを構築する。
    servo_id: 0〜7, angle: 0〜180度
    """
    angle = max(0, min(180, angle))
    header = b'\xAA\x55' + struct.pack('<BBHH', 0x01, 0x02, seq, 4)
    payload = struct.pack('<BBH', 0x01, servo_id, angle)

    crc = crc16_ccitt(header[2:] + payload)
    return header + payload + struct.pack('<H', crc) + b'\x7E'

def build_reset(seq=1):
    """全サーボを中立位置（90度）にリセットするパケットを構築する。"""
    header = b'\xAA\x55' + struct.pack('<BBHH', 0x01, 0x02, seq, 1)
    payload = b'\x03'
    crc = crc16_ccitt(header[2:] + payload)
    return header + payload + struct.pack('<H', crc) + b'\x7E'

# ── メイン ────────────────────────────────────────────────

PORT  = 'COM5'    # 環境に合わせて変更
BAUD  = 921600
INTERVAL = 0.05   # コマンド間隔（秒）

ser = serial.Serial(PORT, BAUD, timeout=0.5)
time.sleep(0.5)
seq = 1

try:
    # ── 例1: 全8本を一括して任意位置へ ────────────────────
    print("=== 例1: 全サーボを任意位置へ一括設定 ===")
    # [servo0=45, servo1=135, servo2=60, servo3=120, servo4=90, servo5=90, servo6=30, servo7=150]
    target = [45, 135, 60, 120, 90, 90, 30, 150]
    pkt = build_set_all(target, seq)
    ser.write(pkt)
    print(f"送信 (seq={seq}): {target}")
    seq += 1
    time.sleep(INTERVAL)

    # ── 例2: 左右対称で動かす ──────────────────────────────
    print("\n=== 例2: 左右対称ポーズ ===")
    poses = [
        [60, 120, 70, 110, 80, 100, 90, 90],   # ポーズA
        [90,  90, 90,  90, 90,  90, 90, 90],   # 中立
        [120,  60, 110, 70, 100, 80, 90, 90],  # ポーズB（左右反転）
    ]
    for pose in poses:
        pkt = build_set_all(pose, seq)
        ser.write(pkt)
        print(f"  seq={seq}: {pose}")
        seq += 1
        time.sleep(0.5)   # ポーズ間は0.5秒待機

    # ── 例3: 個別サーボを順番に動かすウェーブ動作 ─────────
    print("\n=== 例3: ウェーブ動作（サーボ0〜7を順番に動かす） ===")
    current = [90] * 8
    for i in range(8):
        current[i] = 130
        pkt = build_set_all(current, seq)
        ser.write(pkt)
        print(f"  seq={seq}: servo[{i}]=130")
        seq += 1
        time.sleep(0.15)
        current[i] = 90  # 元に戻す
        pkt = build_set_all(current, seq)
        ser.write(pkt)
        seq += 1
        time.sleep(0.1)

    # ── 例4: テキスト通信と同様の単一サーボ制御 ───────────
    print("\n=== 例4: 単一サーボ制御（SET_SERVO, cmd=0x01） ===")
    for servo_id, angle in [(0, 45), (3, 135), (7, 60)]:
        pkt = build_set_single(servo_id, angle, seq)
        ser.write(pkt)
        print(f"  seq={seq}: servo[{servo_id}]={angle}")
        seq += 1
        time.sleep(INTERVAL)

    # ── 最後に全サーボをリセット ───────────────────────────
    print("\n=== 全サーボリセット（90度） ===")
    ser.write(build_reset(seq))
    print("  リセット送信完了")

finally:
    ser.close()
    print("\n接続を閉じました")
```

### PowerShellによる SET_ALL 送信例

PowerShell 5.1 以降であれば `struct.pack` 相当の処理が可能です。

```powershell
function Build-SetAll {
    param([int[]]$Positions, [int]$Seq = 1)

    function CRC16-CCITT([byte[]]$data) {
        $crc = 0xFFFF
        foreach ($b in $data) {
            $crc = $crc -bxor ($b -shl 8)
            for ($j = 0; $j -lt 8; $j++) {
                if ($crc -band 0x8000) { $crc = (($crc -shl 1) -bxor 0x1021) -band 0xFFFF }
                else                  { $crc = ($crc -shl 1) -band 0xFFFF }
            }
        }
        return $crc
    }

    $header = [byte[]](0xAA, 0x55, 0x01, 0x02,
                       ($Seq -band 0xFF), (($Seq -shr 8) -band 0xFF),
                       17, 0)            # LEN=17, リトルエンディアン

    $payload = [byte[]](0x02)            # CMD=SET_ALL
    foreach ($p in $Positions) {
        $p = [Math]::Max(0, [Math]::Min(180, $p))
        $payload += [byte]($p -band 0xFF)
        $payload += [byte](($p -shr 8) -band 0xFF)
    }

    $crcData = $header[2..($header.Length-1)] + $payload
    $crc = CRC16-CCITT $crcData
    $crcBytes = [byte[]](($crc -band 0xFF), (($crc -shr 8) -band 0xFF))

    return $header + $payload + $crcBytes + [byte[]](0x7E)
}

$port = New-Object System.IO.Ports.SerialPort COM5,921600,None,8,one
$port.Handshake = [System.IO.Ports.Handshake]::None
$port.Open()

# 全8本を異なる角度に設定
$pkt = Build-SetAll -Positions @(45, 135, 60, 120, 90, 90, 30, 150) -Seq 1
$port.Write($pkt, 0, $pkt.Length)
Write-Host "送信: $($pkt | ForEach-Object { '{0:X2}' -f $_ }) -join ' ')"
Start-Sleep -Milliseconds 100

# 全サーボを中立（90度）に戻す
$pkt = Build-SetAll -Positions @(90,90,90,90,90,90,90,90) -Seq 2
$port.Write($pkt, 0, $pkt.Length)

$port.Close()
```

### サーボ値の目安

| 値 | 意味 |
|----|------|
| `0`  | 最大左（または最小角） |
| `90` | 中立（デフォルト） |
| `180`| 最大右（または最大角） |

- 有効範囲: **0〜180**（範囲外は自動クランプ）
- ID: **0〜7**（8本）
- SET_ALL は全8本を同時更新、SET_SERVO は1本のみ変更してそのほかは保持

---

最終更新: 2026-04-07
