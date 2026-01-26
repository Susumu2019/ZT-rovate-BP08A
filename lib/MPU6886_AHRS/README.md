# MPU6886 AHRS ライブラリ

Madgwick方向フィルタを搭載したMPU6886 6軸IMUの移植可能なライブラリです。

## 機能

- ✅ MPU6886センサドライバ（加速度計、ジャイロ、温度）
- ✅ Madgwick AHRS方向フィルタ
- ✅ 自動ジャイロバイアス校正
- ✅ リアルタイムdt計算による正確な積分
- ✅ I2Cバス共有サポート（他のセンサとの併用可）
- ✅ プラットフォーム非依存（Arduino互換）
- ✅ 使いやすい統一API

## ファイル構成

- `MPU6886.h/cpp` - 低レベルセンサドライバ
- `MadgwickAHRS.h/cpp` - 方向フィルタ
- `MPU6886_AHRS.h/cpp` - 高レベル統一インターフェース

## インストール手順

### 1. ハードウェア接続

**M5CoreS3SE + MPU6886の接続（PORT.A使用）**

| MPU6886 | M5CoreS3SE | GPIO |
|---------|-----------|------|
| VCC | 3.3V | - |
| GND | GND | - |
| SDA | PORT.A SDA | GPIO2 |
| SCL | PORT.A SCL | GPIO1 |
| INT | GPIO39 | GPIO39 |

### 2. ライブラリのインストール

```bash
# platformio.iniでlib_depsに追加
lib_deps = 
    M5Unified
    MPU6886_AHRS
```

### 3. プログラムでの初期化

```cpp
#include <Wire.h>
#include "MPU6886_AHRS.h"

MPU6886_AHRS imu;

void setup() {
  Serial.begin(115200);
  
  // M5CoreS3SE のPORT.A I2C初期化
  Wire1.begin(2, 1);  // SDA=GPIO2, SCL=GPIO1
  
  // IMU初期化
  if (imu.begin(&Wire1, 0x68, 100.0f, 0.4f) == 0) {
    Serial.println("IMU初期化成功");
  } else {
    Serial.println("IMU初期化失敗");
    while(1) delay(100);
  }
  
  // ジャイロバイアス校正（デバイスを静止させて実行）
  Serial.println("ジャイロ校正中...動かないでください!");
  imu.calibrateGyro(200);
  Serial.println("校正完了");
}
```

## クイックスタート

### 基本的な使用方法

```cpp
#include <Wire.h>
#include "MPU6886_AHRS.h"

MPU6886_AHRS imu;

void setup() {
  Serial.begin(115200);
  Wire.begin(2, 1);  // SDA=2, SCL=1
  
  // IMU初期化（自動キャリブレーション対応）
  if (imu.begin(&Wire) == 0) {
    Serial.println("IMU OK");
    imu.calibrateGyro(200);  // 校正（デバイスを静止させてください！）
  }
}

void loop() {
  imu.update();
  
  Serial.printf("Roll: %.1f, Pitch: %.1f, Yaw: %.1f\n",
                imu.getRoll(), imu.getPitch(), imu.getYaw());
  
  delay(10);
}
```

### 共有I2Cバスの使用

```cpp
// Wire1 を MPU6886用、Wire を他のセンサ用として使用
Wire.begin(21, 22);   // 他のセンサ
Wire1.begin(2, 1);    // MPU6886

MPU6886_AHRS imu;
imu.begin(&Wire1, 0x68);  // Wire1 を明示的に指定
```

### 高度な使用方法

```cpp
// 低レベルセンサへのアクセス
float ax, ay, az;
imu.sensor().readAccel(&ax, &ay, &az);
imu.sensor().setAccelScale(MPU6886::AFS_4G);

// フィルタへのアクセス
imu.filter().setGain(0.5f);  // 収束速度を調整
imu.filter().reset();        // 方向をリセット

// 手動バイアス補正
float bx, by, bz;
imu.getGyroBias(&bx, &by, &bz);
imu.setGyroBias(0.1, -0.05, 0.02);
```

## Madgwickフィルタについて

### フィルタの動作原理

Madgwick AHRS フィルタは、加速度計とジャイロスコープの両方のデータを融合して、正確な3D方向（Roll/Pitch/Yaw）を計算します。

**利点：**
- ジャイロの統合ドリフトを加速度計で補正
- 磁気センサなしでヨー角を推定可能
- 低い計算負荷で高速処理
- ノイズに対してロバスト

### ゲイン（Beta）チューニング

| ゲイン値 | 特性 | 用途 |
|---------|------|------|
| 0.1-0.2 | 遅い、スムーズ | ノイズが多い環境 |
| 0.3-0.4 | バランス型 | **推奨（汎用）** |
| 0.5-0.6 | 高速、反応良 | 高精度が必要な場合 |
| 0.7以上 | 非常に高速 | リアルタイム性重視 |

**チューニング例：**

```cpp
// 遅い応答が必要な場合
imu.filter().setGain(0.2f);

// 高速な追従が必要な場合
imu.filter().setGain(0.6f);
```

### フィルタの初期化と管理

```cpp
// フィルタのリセット（方向を0にする）
imu.filter().reset();

// サンプルレートの設定（デフォルト: 100Hz）
imu.filter().begin(100.0f);

// 四元数の取得（内部表現）
float w, x, y, z;
imu.filter().getQuaternion(&w, &x, &y, &z);
```

### キャリブレーション

**ジャイロバイアス校正：**

```cpp
// 200サンプル分のキャリブレーション（推奨）
imu.calibrateGyro(200);

// 校正値の取得
float bx, by, bz;
imu.getGyroBias(&bx, &by, &bz);
Serial.printf("Gyro bias: X=%.3f Y=%.3f Z=%.3f\n", bx, by, bz);
```

## API リファレンス

### MPU6886_AHRS（高レベル）

| メソッド | 説明 |
|---------|------|
| `begin(wire, addr, rate, gain)` | 初期化（デフォルト: Wire, 0x68, 100Hz, 0.4） |
| `calibrateGyro(samples)` | ジャイロバイアス校正（デフォルト: 200サンプル） |
| `update()` | 方向を更新（ループ内で呼び出し） |
| `getRoll/Pitch/Yaw()` | 方向を取得（度） |
| `getAccel/Gyro/Temp()` | センサデータを取得 |
| `resetOrientation()` | 方向をリセット |

### MPU6886（低レベル）

| メソッド | 説明 |
|---------|------|
| `begin(wire, addr)` | センサ初期化 |
| `readAccel/Gyro/Temp()` | キャリブレーション済みデータを読込 |
| `readAccelADC/GyroADC()` | 生のADC値を読込 |
| `setAccelScale()` | 加速度計レンジを設定（2/4/8/16g） |
| `setGyroScale()` | ジャイロレンジを設定（250/500/1000/2000 dps） |

### MadgwickAHRS（フィルタ）

| メソッド | 説明 |
|---------|------|
| `begin(sampleRateHz)` | サンプルレートを設定 |
| `update(gx,gy,gz, ax,ay,az, dt)` | センサデータで更新 |
| `getRoll/Pitch/Yaw()` | オイラー角を取得（度） |
| `getQuaternion()` | 四元数を取得（w,x,y,z） |
| `setGain(beta)` | フィルタゲインを設定（0.1-0.5） |
| `reset()` | 初期化状態にリセット |

## センサ設定

### センサレンジの設定

```cpp
// 加速度計: AFS_2G, AFS_4G, AFS_8G, AFS_16G
imu.sensor().setAccelScale(MPU6886::AFS_8G);

// ジャイロ: GFS_250DPS, GFS_500DPS, GFS_1000DPS, GFS_2000DPS
imu.sensor().setGyroScale(MPU6886::GFS_2000DPS);
```

## 他のプラットフォームへの移植

このライブラリはArduino `Wire` ライブラリを使用しています。移植には以下の置き換えが必要です：

1. `TwoWire*` をプラットフォームのI2Cハンドルに置き換え
2. `micros()` をマイクロ秒タイマーに置き換え
3. `delay()` をプラットフォームのdelay関数に置き換え

## ライセンス

MIT License - 商用・個人利用共に無料

## クレジット

- Madgwickアルゴリズム: Sebastian O.H. Madgwick
- MPU6886データシート: TDK InvenSense