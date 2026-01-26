#ifndef CONFIG_H
#define CONFIG_H

// IOピン番号
#define SDA_PIN            2 /*I2C SDAピン*/
#define SCL_PIN            1 /*I2C SCLピン*/

//デバイス情報
static constexpr const char* device_name = "rovate";   /*デバイス名*/
static constexpr const char* ver = "0.1.0";  /*バージョン*/

#define LCD_WIDTH        320 /*CoreS3SE LCD幅*/
#define LCD_HEIGHT       240 /*CoreS3SE LCD高さ*/

// 通信設定（必要に応じて変更してください）
// WiFi (STA) 接続情報
constexpr const char* WIFI_SSID = "w20240812-g";
constexpr const char* WIFI_PASSWORD = "AT2dYrkMCp6Q";

// UDP 送信先
static constexpr uint8_t UDP_TARGET_IP0 = 192;
static constexpr uint8_t UDP_TARGET_IP1 = 168;
static constexpr uint8_t UDP_TARGET_IP2 = 0;
static constexpr uint8_t UDP_TARGET_IP3 = 100;
constexpr uint16_t UDP_TARGET_PORT = 5000;

// シリアル送信設定（UART2 を想定。TX/RX ピンは環境に合わせて調整）
static constexpr uint32_t SERIAL_BAUD = 921600;
static constexpr int SERIAL_TX_PIN = 17;
static constexpr int SERIAL_RX_PIN = 18;

// 制御送信レート（Hz）
static constexpr uint16_t CONTROL_RATE_HZ = 100; // 100Hz 推奨（BLE/UDP両対応）

#endif // CONFIG_H