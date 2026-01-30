
/**
 ****************************************************************************
 * @file     main.cpp
 * @brief    main for Source File
 * @version  V0.9
 * @date     20XX-XX-XX
 * @author   XXXXXX
 *****************************************************************************
 */
#include <Arduino.h>
#include <M5CoreS3.h>
#include "App/AppManager/AppManager.h"
#include "system/touch/TouchManager.h"
#include "timer/timer.h"
#include "system/system.h"
#include "MPU6886_AHRS.h"
#include "UI/Icon/Icon.h"
#include "config.h"
#include <FastLED.h>
#include <SD.h>
#include <esp_sleep.h>
#include <Adafruit_PWMServoDriver.h>
// Comm
#include "system/comm/UdpSender.h"
#include "system/comm/SerialSender.h"
#include "system/Settings.h"

#include <WiFiUdp.h>

// WS2812設定
#define WS2812_PIN 6
#define WS2812_COUNT 16
CRGB leds[WS2812_COUNT];

// ロゴ画像のサイズ（rovate.png.h の生成サイズ）
static constexpr int LOGO_WIDTH = 240;
static constexpr int LOGO_HEIGHT = 240;

float imu_roll_offset = 0.0f;
// サーボ・LED制御開始フラグ
bool systemStarted = false;
float imu_pitch_offset = 0.0f;
float imu_yaw_offset = 0.0f;
bool imu6886_connected = false;
WiFiUDP udpReceiver;
constexpr uint16_t UDP_LISTEN_PORT = 12345;
uint8_t udpRecvBuf[128];
uint16_t g_seq = 0;
uint16_t g_servoPos[8] = {90, 90, 90, 90, 90, 90, 90, 90};
uint16_t g_servoOff[8] = {0};
bool pca9685_connected = false;
Adafruit_PWMServoDriver pwmDriver(0x40);
UdpSender udpSender;
SerialSender serialSender;

// グローバルオブジェクト
M5Canvas canvas(&M5.Lcd);
AppManager appManager;
MPU6886_AHRS imu6886_ahrs;  // 外部参照可能にするためstaticを削除

/**
 * @brief サーボをフリー（PWM出力OFF）にする
 */
void setServoFree() {
	if (!pca9685_connected) return;
	for (int ch = 0; ch < 8; ++ch) {
		pwmDriver.setPWM(ch, 0, 0); // PWM出力OFF
	}
}

// IMU6886センサーデータ構造体
struct IMU6886Data {
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    uint8_t temp;
};
IMU6886Data imu6886Data = {0};

/**
 * @brief PCA9685へ現在のサーボ値を反映（micro秒指定 500-2500）
 */
static void applyServoOutputs() {
	if (!pca9685_connected) return;
	for (int ch = 0; ch < 8; ++ch) {
		// g_servoPos[ch]は角度(0～180)で格納されているのでパルス幅(μs)に変換
		uint16_t angle = g_servoPos[ch];
		uint16_t pulse = 500 + (angle * 2000) / 180; // 0度=500μs, 90度=1500μs, 180度=2500μs
		pulse += g_servoOff[ch];
		if (pulse < 500) pulse = 500;
		if (pulse > 2500) pulse = 2500;

		// 後ろ4chは反転（AppManualと合わせる）
		if (ch >= 4) {
			// 500-2500 を反転
			pulse = 3000 - pulse;  // 500→2500, 2500→500
		}

		// 50Hzでのカウント値に変換 (0-4095)
		uint16_t count = (pulse * 4096) / 20000;  // 20ms周期
		pwmDriver.setPWM(ch, 0, count);
	}
}

// 受信パケットの簡易パース（g_servoPosを更新）
void processUdpServoPacket(const uint8_t* data, size_t len) {
	// 例: 先頭2バイト=SYNC(0xAA55), その後8ch分のu16(16バイト)
	if (len < 2 + 16) return;
	if (data[0] != 0xAA || data[1] != 0x55) return;
	for (int i = 0; i < 8; ++i) {
		uint16_t angle = data[2 + i * 2] | (data[2 + i * 2 + 1] << 8);
		if (angle > 180) angle = 180;
		if (angle < 0) angle = 0;
		g_servoPos[i] = angle;
	}
	// g_servoPos[]の内容をシリアル出力
	Serial.print("g_servoPos: [");
	for (int i = 0; i < 8; ++i) {
		Serial.print(g_servoPos[i]);
		if (i < 7) Serial.print(", ");
	}
	Serial.println("]");
	applyServoOutputs();
}

/**
 * @brief LEDパターンを通信状態に合わせて更新
 * @param udpOk UDP送信成功
 * @param serialOk シリアル送信成功
 */
void updateLedPattern(bool udpOk, bool serialOk) {
		// 26個LEDのうち中央3個(11,12,13)を緑、それ以外は白
		for (int i = 0; i < WS2812_COUNT; i++) {
			if (i == 0) {
				leds[i] = CRGB::Red;
			} else if (i == WS2812_COUNT - 1) {
				leds[i] = CRGB::Blue;
			} else if (i == 6 || i == 7 || i == 8) {
				leds[i] = CRGB::Green;
			} else {
				leds[i] = CRGB::White;
			}
		}
		FastLED.show();
}



void setup() {
	Serial.begin(115200);
	delay(500);
	
	// Wire ライブラリからの I2C エラーログを抑制
	esp_log_level_set("Wire", ESP_LOG_NONE);
	
	// M5CoreS3の初期化
	auto cfg = M5.config();
	CoreS3.begin(cfg);
	
	// バッテリー充電を常に有効化（電源オン・オフ問わず充電継続）
	M5.Power.setBatteryCharge(true);
	M5.Power.setChargeCurrent(500);  // 充電電流 500mA（安全な範囲で設定）
	
	// SDカードをマウント（CoreS3のSDピンを明示）
	SPI.begin(GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_11, GPIO_NUM_4);
	bool sdReady = SD.begin(GPIO_NUM_4, SPI);
	

	// WS2812の初期化
	FastLED.addLeds<WS2812, WS2812_PIN, GRB>(leds, WS2812_COUNT);
	FastLED.setBrightness(255);
	
	// 起動時は全LEDを黄色で点灯
	for (int i = 0; i < WS2812_COUNT; i++) {
		leds[i] = CRGB::Yellow;
	}
	FastLED.show();
	// サーボをOFF（フリー）
	setServoFree();
	
	// 各システムの初期化
	publicTimer.begin();    // タイマー初期化
	
	// PORT.A I2C初期化 (Wire: SDA=GPIO2, SCL=GPIO1) - 外部デバイス/IMU用
	Wire.begin(SDA_PIN, SCL_PIN);
	delay(50);

	// PCA9685 初期化（サーボ駆動用）
	if (pwmDriver.begin()) {
		pca9685_connected = true;
		pwmDriver.setOscillatorFrequency(25000000);
		pwmDriver.setPWMFreq(50);
		applyServoOutputs();  // 初期位置を反映
		Serial.println("PCA9685: ready (50Hz)");
	} else {
		pca9685_connected = false;
		Serial.println("PCA9685: not found");
	}
    
	// サブI2C初期化 (Wire1: SDA=GPIO21, SCL=GPIO22) - 内部バス用
	Wire1.begin(21, 22);
	delay(100);
	
	// 初回データ読み込み（IMU接続時のみ）
	if (imu6886_connected) {
		imu6886_ahrs.update();
		// IMU初期値（オフセット）を記録
		imu_roll_offset = imu6886_ahrs.getRoll();
		imu_pitch_offset = imu6886_ahrs.getPitch();
		imu_yaw_offset = imu6886_ahrs.getYaw();
	}

	// 画面描画の準備
	canvas.fillScreen(WHITE);
	canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
	

	// rovateロゴの表示（3秒間）
	M5.Lcd.fillScreen(WHITE);
	if (sdReady) {
		const int logoX = (LCD_WIDTH - LOGO_WIDTH) / 2;
		const int logoY = (LCD_HEIGHT - LOGO_HEIGHT) / 2;
		File logo = SD.open("/rovate_240_240.bmp", FILE_READ);
		if (logo) {
			M5.Lcd.drawBmp(&logo, logoX, logoY);
			logo.close();
		}
	} else {
		M5.Lcd.setTextColor(WHITE, BLACK);
		M5.Lcd.setTextDatum(MC_DATUM);
		M5.Lcd.drawString("SD NG", LCD_WIDTH / 2, LCD_HEIGHT / 2);
	}

	// WiFi/UDP初期化（ロゴ表示中に実施）
	if (Settings::getInstance().isWifiEnabled()) {
		udpSender.begin();
		udpReceiver.begin(UDP_LISTEN_PORT); // UDP受信も開始
	}

	// 起動から3秒以上経過していなければロゴを表示したまま待機
	uint32_t startMs = millis();
	while (millis() - startMs < 3000) {
		if (imu6886_connected) imu6886_ahrs.update();
		delay(10);
	}

	// IMU6886初期化
	int imu_init_result = imu6886_ahrs.begin(&Wire, 0x68, 100.0f, 0.1f);	
	if (imu_init_result == 0) {
		imu6886_connected = true;
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.setCursor(10, 100);
		M5.Lcd.setTextSize(2);
		M5.Lcd.setTextColor(TFT_YELLOW);
		M5.Lcd.print("Calibrating...");
		M5.Lcd.setCursor(10, 130);
		M5.Lcd.print("Keep still!");
		
		imu6886_ahrs.calibrateGyro(500);
		M5.Lcd.fillScreen(BLACK);
	} else {
		imu6886_connected = false;
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.setCursor(10, 100);
		M5.Lcd.setTextSize(2);
		M5.Lcd.setTextColor(TFT_RED);
		M5.Lcd.print("IMU NOT FOUND");
		M5.Lcd.setCursor(10, 130);
		M5.Lcd.setTextColor(TFT_YELLOW);
		M5.Lcd.print("Starting anyway...");
		delay(2000);
	}
	
	delay(100);

	// ここでIMUオフセットを記録（値が安定したタイミング）
	if (imu6886_connected) {
		imu6886_ahrs.update();
		imu_roll_offset = imu6886_ahrs.getRoll();
		imu_pitch_offset = imu6886_ahrs.getPitch();
		imu_yaw_offset = imu6886_ahrs.getYaw();
		Serial.println("IMU offset set after logo.");
	}

	// ここで画面を黒くする
	M5.Lcd.fillScreen(BLACK);


	// アプリマネージャの初期化
	appManager.initializeApps();
	// 起動直後はHome画面を表示
	appManager.showHomeScreen();

	// 設定システムの初期化（NVSから読み込み）
	Settings::getInstance().begin();

	// 通信初期化（WiFi/UDPとシリアルの排他制御）
	if (Settings::getInstance().isWifiEnabled()) {
		udpSender.begin();
		// WiFi有効時はシリアル制御を強制OFF
		Settings::getInstance().setSerialEnabled(false);
	} else {
		// WiFi無効時のみシリアル制御をON
		Settings::getInstance().setSerialEnabled(true);
		serialSender.begin();
	}
}

void sendImuUdp(float ax, float ay, float az, float gx, float gy, float gz, uint8_t temp) {
	// PC側の受信形式: [AA 55][roll][pitch][yaw][gx][gy][gz][temp] (float*6+uint8)
	uint8_t buf[2 + 4*6 + 1];
	buf[0] = 0xAA; buf[1] = 0x55;
	memcpy(&buf[2],  &ax, 4);   // roll
	memcpy(&buf[6],  &ay, 4);   // pitch
	memcpy(&buf[10], &az, 4);   // yaw
	memcpy(&buf[14], &gx, 4);   // gx
	memcpy(&buf[18], &gy, 4);   // gy
	memcpy(&buf[22], &gz, 4);   // gz
	buf[26] = temp;
	IPAddress pc_broadcast(192,168,0,255);
	udpSender.sendImuPacket(buf, sizeof(buf), pc_broadcast, 12346);
}

void loop() {
	// loop開始時に1回だけ通常制御へ切り替え
	if (!systemStarted) {
		// LEDを通常表示に
		updateLedPattern(true, true); // 通常表示（引数は任意、必要に応じて変更）
		// サーボ制御ON
		applyServoOutputs();
		systemStarted = true;
	}
	// ハードウェアの状態を更新（ボタン、電源など）
	CoreS3.update();

	// --- UDP受信: サーボ制御コマンド ---
	int packetSize = udpReceiver.parsePacket();
	if (packetSize > 0 && packetSize <= (int)sizeof(udpRecvBuf)) {
		int len = udpReceiver.read(udpRecvBuf, sizeof(udpRecvBuf));
		if (len > 0) {
			processUdpServoPacket(udpRecvBuf, len);
		}
	}
	
	// 電源ボタン長押しで電源オフ（3秒以上）
	if (M5.BtnPWR.pressedFor(3000)) {
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.setTextColor(WHITE);
		M5.Lcd.setTextDatum(MC_DATUM);
		M5.Lcd.drawString("Power Off...", LCD_WIDTH / 2, LCD_HEIGHT / 2);
		delay(500);
		M5.Power.powerOff();
		esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);  // 全ての自動ウェイクを無効化
		esp_deep_sleep_start();  // 完全停止（電源ボタンでのみ復帰）
	}
	
	// タッチパネルの状態を更新（座標取得と基本フラグのみ）
	touchManager.update();
	
	// === IMU センサー更新（IMU接続時のみ） ===
	if (imu6886_connected) {
		imu6886_ahrs.update();
	}
	
	// シリアルコマンド受信処理（アプリloopより前に実行！）
	if (Settings::getInstance().isSerialEnabled()) {
		bool updated = false;
		if (Settings::getInstance().getSerialMode() == Settings::SERIAL_TEXT) {
			updated = serialSender.processTextCommand(g_servoPos, g_servoOff);
		} else if (Settings::getInstance().getSerialMode() == Settings::SERIAL_BINARY) {
			updated = serialSender.processBinaryCommand(g_servoPos, g_servoOff);
		}
		if (updated) {
			applyServoOutputs();
		}
	}

	// ボタンB（物理ボタン）が離されたらホーム画面を表示
	if (M5.BtnB.wasReleased()) {
		appManager.showHomeScreen();
	}

	// 現在のアプリのメインロジック実行
	appManager.loop();
	
	// 画面に描画
	appManager.draw(canvas);
	canvas.pushSprite(&M5.Lcd, 0, 0);

	// --- 制御データ送信（UDP / Serial 並行）---
	static uint32_t lastSendMs = 0;
	const uint32_t intervalMs = 1000 / CONTROL_RATE_HZ;
	uint32_t nowMs = millis();
	if (nowMs - lastSendMs >= intervalMs) {
		lastSendMs = nowMs;
		float roll=0, pitch=0, yaw=0, gx=0, gy=0, gz=0;
		float ax=0, ay=0, az=0;
		uint8_t t8 = 0;
		if (imu6886_connected) {
			roll = imu6886_ahrs.getRoll();
			pitch = imu6886_ahrs.getPitch();
			yaw = imu6886_ahrs.getYaw();
			imu6886_ahrs.getGyro(&gx, &gy, &gz);
			imu6886_ahrs.getAccel(&ax, &ay, &az);
			float tf = imu6886_ahrs.getTemperature();
			if (tf < 0) tf = 0; if (tf > 255) tf = 255; t8 = (uint8_t)(tf);
		}
		// オフセット補正
		float roll_deg = roll - imu_roll_offset;
		float pitch_deg = pitch - imu_pitch_offset;
		float yaw_deg = yaw - imu_yaw_offset;
		// デバッグ: 送信値をシリアル出力
		Serial.printf("IMU_SEND: roll=%.2f pitch=%.2f yaw=%.2f gx=%.2f gy=%.2f gz=%.2f temp=%d\n", roll_deg, pitch_deg, yaw_deg, gx, gy, gz, t8);
		sendImuUdp(roll_deg, pitch_deg, yaw_deg, gx, gy, gz, t8);
			// ボタンA（物理ボタン）でIMU初期値（オフセット）再設定
			// ボタンA長押し（2秒以上）でのみキャリブレーション実行
			if (M5.BtnA.pressedFor(2000)) {
				if (imu6886_connected) {
					// サーボをフリーに
					setServoFree();
					M5.Lcd.fillScreen(BLACK);
					M5.Lcd.setCursor(10, 100);
					M5.Lcd.setTextSize(2);
					M5.Lcd.setTextColor(TFT_YELLOW);
					M5.Lcd.print("Calibrating...");
					M5.Lcd.setCursor(10, 130);
					M5.Lcd.print("Keep still!");
					imu6886_ahrs.calibrateGyro(500);
					delay(500); // キャリブ後少し待つ
					imu6886_ahrs.update();
					imu_roll_offset = imu6886_ahrs.getRoll();
					imu_pitch_offset = imu6886_ahrs.getPitch();
					imu_yaw_offset = imu6886_ahrs.getYaw();
					M5.Lcd.fillScreen(BLACK);
					Serial.println("IMU offset updated & gyro calibrated.");
					// サーボ制御値を復帰
					applyServoOutputs();
				}
			}
		// ここで g_servoPos / g_servoOff を必要に応じて更新してください（AppManual連携想定）
		// IMU出力設定を取得（OFFの場合は送信自体をスキップ）
		bool imuOutputEnabled = Settings::getInstance().isImuOutputEnabled();
		bool udpOk = false;
		bool serialOk = false;
		// IMU出力ONの時のみデータ送信
		if (imuOutputEnabled) {
			// UDP送信（有効時のみ）
			if (Settings::getInstance().isWifiEnabled()) {
				udpOk = udpSender.sendControl(ax, ay, az, gx, gy, gz, t8, g_servoPos, g_servoOff, g_seq, true);
			}
			// シリアル送信（有効時のみ、モード切り替え）
			if (Settings::getInstance().isSerialEnabled()) {
				if (Settings::getInstance().getSerialMode() == Settings::SERIAL_TEXT) {
					serialOk = serialSender.sendControlText(ax, ay, az, gx, gy, gz, t8, g_servoPos, g_servoOff, g_seq, true);
				} else {
					serialOk = serialSender.sendControl(ax, ay, az, gx, gy, gz, t8, g_servoPos, g_servoOff, g_seq, true);
				}
			}
			g_seq++;
		}
		// TopBarに送信状態を通知
		if (udpOk) appManager.getTopBar().notifyUdpSent();
		if (serialOk) appManager.getTopBar().notifySerialSent();
		// UDP接続状態を更新
		appManager.getTopBar().setUdpConnected(udpSender.isReady());
		// LEDパターンを更新（通信状態を視覚化）
		updateLedPattern(udpOk, serialOk);
	}
}

/*********************************** END OF FILE ******************************/
