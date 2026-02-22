## AppWifi - WiFi/UDP接続画面

最終更新日: 2026年2月1日

---

## 設定手順（初回セットアップ）

WiFi/UDP通信を使用する前の設定ステップです。

### ⚠️ 重要：config.h の作成

**config.h にはWiFiのSSIDとパスワード等の機密情報が含まれるため、リポジトリの .gitignore で除外されています。**

以下の手順で個別の `config.h` ファイルを作成してください：

**ステップ0: config.h ファイルの準備**

1. [include/config.h.template](../../../../include/config.h.template) を `include/config.h` にコピー：
   ```bash
   cd include
   copy config.h.template config.h
   ```

2. または、VS Code で config.h.template を右クリック → 「コピー」して include/ フォルダに貼り付け、ファイル名を `config.h` に変更

### ステップ1: WiFi接続情報の設定

作成した `include/config.h` ファイルを編集して、接続先WiFiネットワークを設定します：

```cpp
// WiFi (STA) 接続情報
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ← 接続先WiFiのSSIDに変更
constexpr const char* WIFI_PASSWORD = "YOUR_PASSWORD";     // ← WiFiパスワードに変更
```

**設定例：**
```cpp
constexpr const char* WIFI_SSID = "home-wifi";
constexpr const char* WIFI_PASSWORD = "mypassword123";
```

⚠️ **注意：** `config.h` には機密情報が含まれるため、絶対に GitHub にコミットしないでください。`.gitignore` で除外されているので自動的には含まれませんが、確認してください。

### ステップ2: UDP送信先の確認・設定

同じく [include/config.h](../../../../include/config.h) で、UDP通信先（PC側）のIPアドレスを設定します（デフォルトは 192.168.0.100）：

```cpp
// UDP 送信先（PC側のIPアドレス）
static constexpr uint8_t UDP_TARGET_IP0 = 192;   // 第1オクテット
static constexpr uint8_t UDP_TARGET_IP1 = 168;   // 第2オクテット
static constexpr uint8_t UDP_TARGET_IP2 = 0;     // 第3オクテット
static constexpr uint8_t UDP_TARGET_IP3 = 100;   // 第4オクテット
constexpr uint16_t UDP_TARGET_PORT = 5000;       // UDPポート
```

**設定例：PC IPアドレスが 192.168.1.50 の場合**
```cpp
static constexpr uint8_t UDP_TARGET_IP0 = 192;
static constexpr uint8_t UDP_TARGET_IP1 = 168;
static constexpr uint8_t UDP_TARGET_IP2 = 1;
static constexpr uint8_t UDP_TARGET_IP3 = 50;
```

**PCのIPアドレス確認方法（Windows）：**
```powershell
ipconfig
# または
ipconfig | findstr "IPv4"
```

### ステップ3: ファームウェア書き込み

1. [include/config.h](../../../../include/config.h) を保存
2. VS Code の PlatformIO パネルで「Build」をクリック
3. 「Upload」をクリックしてM5Stack CoreS3に書き込み
4. デバイスが再起動し、WiFiに自動接続

### ステップ4: WiFi接続状態の確認

デバイスの「WiFi」アプリ画面でWiFi接続状態を確認：
- 接続SSID名
- ロボット側のIPアドレス
- 電波強度（RSSI）
- UDP接続状態（PC接続済み/未接続）

---

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

---

## Hello World: WindowsでUDP通信をテストする

ロボットに接続してサーボを制御し、センサデータをリアルタイム受信する手順です。

### 準備

1. ロボットをWiFiネットワークに接続
2. ロボットのIPアドレスを確認（AppWifiで表示されます）
   - 例：`192.168.0.11`
3. PCがロボットと同じWiFiネットワークに接続
4. デバイスがText or UDPモード（バイナリモードでない）に設定

### 方法1: GUIツール（最も簡単）

```bash
# プロジェクトルートで実行
cd c:\Users\zeate\Documents\PlatformIO\Projects\ZT-rovate-BP08A

# UDP制御GUIを起動
python tools/pc_client/udpcontrol.py
```

**GUIの使い方**
- 上部の「Robot IP」欄にロボットのIPアドレスを入力（例：192.168.0.11）
- スライダーでサーボ0～7を0～180度の範囲で制御
- リアルタイムでIMU値（roll, pitch, yaw等）が表示
- 3D立方体でロボットの姿勢が可視化されます

### 方法2: シンプルなPythonスクリプト（カスタム制御）

以下のコードを `test_udp.py` として保存して実行：

```python
import socket
import struct
import time

# 設定
ROBOT_IP = "192.168.0.11"  # ロボットのIPアドレス
ROBOT_PORT = 12345         # ロボットのUDP受信ポート
LOCAL_PORT = 12346         # PC側の受信ポート

# ソケット作成
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", LOCAL_PORT))
sock.settimeout(1.0)

try:
    # [1] すべてのサーボを中央位置(90度)に設定
    print("=== すべてのサーボを中央位置(90度)に設定 ===")
    servo_angles = [90, 90, 90, 90, 90, 90, 90, 90]
    payload = b'\xAA\x55'  # ヘッダ
    for angle in servo_angles:
        payload += struct.pack('<H', angle)  # uint16 little endian
    
    sock.sendto(payload, (ROBOT_IP, ROBOT_PORT))
    print(f"送信: {payload.hex()}")
    
    # センサデータ受信
    time.sleep(0.1)
    try:
        data, addr = sock.recvfrom(1024)
        print(f"受信({len(data)} bytes): {data.hex()}")
        
        # パース: AA55 + roll(f4) + pitch(f4) + yaw(f4) + gx(f4) + gy(f4) + gz(f4) + temp(u1)
        if len(data) >= 26 and data[0:2] == b'\xAA\x55':
            roll, pitch, yaw, gx, gy, gz = struct.unpack('<ffffff', data[2:26])
            temp = data[26] if len(data) > 26 else 0
            print(f"  roll={roll:.2f}°, pitch={pitch:.2f}°, yaw={yaw:.2f}°")
            print(f"  gx={gx:.2f}°/s, gy={gy:.2f}°/s, gz={gz:.2f}°/s")
            print(f"  temp={temp}°C")
    except socket.timeout:
        print("  受信タイムアウト")
    
    # [2] サーボ0を0度、サーボ1を180度に設定
    print("\n=== サーボ0=0度、サーボ1=180度に設定 ===")
    servo_angles = [0, 180, 90, 90, 90, 90, 90, 90]
    payload = b'\xAA\x55'
    for angle in servo_angles:
        payload += struct.pack('<H', angle)
    
    sock.sendto(payload, (ROBOT_IP, ROBOT_PORT))
    print(f"送信: {payload.hex()}")
    
    time.sleep(0.2)
    try:
        data, addr = sock.recvfrom(1024)
        print(f"受信: {data.hex()}")
        
        if len(data) >= 26 and data[0:2] == b'\xAA\x55':
            roll, pitch, yaw, gx, gy, gz = struct.unpack('<ffffff', data[2:26])
            print(f"  IMU: roll={roll:.2f}°, pitch={pitch:.2f}°, yaw={yaw:.2f}°")
    except socket.timeout:
        print("  受信タイムアウト")
    
    # [3] 連続でセンサデータを受信（3秒間）
    print("\n=== センサデータを連続受信（3秒間） ===")
    start_time = time.time()
    count = 0
    while time.time() - start_time < 3:
        try:
            data, addr = sock.recvfrom(1024)
            if len(data) >= 26 and data[0:2] == b'\xAA\x55':
                roll, pitch, yaw, gx, gy, gz = struct.unpack('<ffffff', data[2:26])
                temp = data[26] if len(data) > 26 else 0
                count += 1
                if count % 5 == 0:  # 5フレームごとに表示
                    print(f"[{count}] roll={roll:7.2f}°, pitch={pitch:7.2f}°, yaw={yaw:7.2f}°, temp={temp}°C")
        except socket.timeout:
            pass
    
    print(f"受信したフレーム数: {count}")

finally:
    sock.close()
    print("\nソケットを閉じました")
```

実行方法：
```bash
# シンプルな実行
python test_udp.py

# もし失敗したら、ロボットのIPアドレスをスクリプト内で確認して修正してください
```

### ネットワーク設定の確認（WiFi接続していない場合）

ロボットのWiFi設定を確認：

1. **デバイス画面で「AppWifi」を開く**
   - IPアドレス、接続状態を確認

2. **接続できていない場合**
   - WiFi SSIDとパスワードを確認・再入力
   - ロボットと同じネットワークにPC接続

### 動作確認ポイント

- **センサデータ受信確認**：roll, pitch, yaw, gx, gy, gzが数値で表示されている
- **サーボ反応確認**：スライダーを動かすと画面上で反応そのピッチが変わる
- **リアルタイム性**：GUIで3D立方体がスムーズに回転する

### トラブルシューティング

**接続できない**
- IPアドレスが正しいか確認（AppWifi画面で確認可能）
- PCがロボットと同じWiFiネットワークに接続しているか確認
- ファイアウォール設定でUDP通信が許可されているか確認
- ロボットのUDP受信ポート（デフォルト12345）を確認

**データが受信できない**
- ロボットの電源確認
- WiFi接続状態の確認
- シリアル通信ツール（テキストモード）で動作確認

---

最終更新: 2026年2月18日
