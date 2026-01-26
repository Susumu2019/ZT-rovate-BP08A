## AppWifi - WiFi/UDP接続画面

### 🏷 役割
WiFi接続・UDP通信の状態表示や制御を行うアプリ画面です。

### 📁 ファイル
- `AppWifi.h` - クラス定義
- `AppWifi.cpp` - 実装

### 使い方
```cpp
appManager.registerApp(new AppWifi());
```

### カスタマイズ例
- WiFi接続状態表示
- UDP送信/受信状態表示
- タッチでWiFi再接続やUDP送信

`AppWifi.cpp` の `handleTouch()` でWiFi/UDP制御ロジックを実装します。
