#include "AppManual.h"
#include "config.h"
#include "system/system.h"
#include "UI/Icon/Icon.h"
#include "timer/timer.h"
#include <cstdio>
#include "UI/Button/Button.h"
#include <Adafruit_PWMServoDriver.h>
#include <Preferences.h>

static Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
static Preferences prefs;

// PCA9685接続状態フラグ（外部定義）
extern bool pca9685_connected;

static const int TOPBAR_HEIGHT = 24;
static const int SCREEN_WIDTH = 320;
static const int SCREEN_HEIGHT = 240;

/**
 * @file AppManual.cpp
 * @brief Manual モード用のアプリ（PCA9685 で 8個のサーボを制御）
 * 
 * PCA9685 PWM ドライバを使用して、最大 8 個のサーボモーターを制御します。
 * 各サーボの角度（0～180度）をスライダーで調整し、リアルタイムに反映できます。
 */

AppManual::AppManual() : _selected_servo(0) {}

void AppManual::setup() {
    mode = 800;

    // 補正値をNVSから読み込む
    loadOffsets();

    // PCA9685 の初期化
    // デフォルト I2C アドレス: 0x40
    // I2C ピンは config.h で定義（SDA_PIN=2, SCL_PIN=1）
    bool pwm_result = pwm.begin();
    if (!pwm_result) {
        pca9685_connected = false;
    } else {
        pca9685_connected = true;
        // PWM 周波数を 50Hz に設定（サーボ制御用）
        pwm.setOscillatorFrequency(25000000);  // 25MHz 内部クロック
        pwm.setPWMFreq(50);  // 50Hz
        
        // 全サーボを初期位置（90度）に設定
        for (int i = 0; i < SERVO_COUNT; i++) {
            setServoAngle(i, 90);
            _servo_positions[i] = 90;
        }
    }
    delay(100);
}

void AppManual::setServoAngle(int channel, int angle) {
    if (channel < 0 || channel >= SERVO_COUNT) return;
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // ユーザー指定角度を保持（表示用）
    _servo_positions[channel] = angle;

    // 中立補正を適用
    int adj_angle = angle + _servo_offsets[channel];
    if (adj_angle < 0) adj_angle = 0;
    if (adj_angle > 180) adj_angle = 180;

    // S4, S5, S6, S7は制御方向を反転
    if (channel >= 4) {
        adj_angle = 180 - adj_angle;
    }

    // 角度をパルス幅にマップ（0度: 375, 180度: 2400）
    int pulse_length = SERVO_MIN_PULSE + (adj_angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
    
    // 1MHz クロック、周波数 50Hz でのカウント値計算
    // pulse_length は microseconds → 4096 段階での値に変換
    uint16_t pwm_value = (pulse_length * 4096) / 20000;  // 20000us = 1/50Hz
    
    pwm.setPWM(channel, 0, pwm_value);
}

void AppManual::testAllServos() {
    // 全サーボを 0 度、90 度、180 度と順に動作確認
    for (int angle = 0; angle <= 180; angle += 90) {
        for (int i = 0; i < SERVO_COUNT; i++) {
            setServoAngle(i, angle);
        }
        delay(500);
    }
    // 全サーボを 90 度に戻す
    for (int i = 0; i < SERVO_COUNT; i++) {
        setServoAngle(i, 90);
    }
}

void AppManual::loadOffsets() {
    prefs.begin("servo", true);
    for (int i = 0; i < SERVO_COUNT; i++) {
        char key[8];
        std::sprintf(key, "off%d", i);
        _servo_offsets[i] = prefs.getInt(key, 0);
    }
    prefs.end();
}

void AppManual::saveOffsets() {
    prefs.begin("servo", false);
    for (int i = 0; i < SERVO_COUNT; i++) {
        char key[8];
        std::sprintf(key, "off%d", i);
        prefs.putInt(key, _servo_offsets[i]);
    }
    prefs.end();
}

bool AppManual::areAllServosAtNeutral() const {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (_servo_positions[i] != 90) {
            return false;
        }
    }
    return true;
}

void AppManual::loop() {
    // Manual モード範囲内の基本更新
    if (mode >= 800 && mode < 900) {
        // サーボの状態を定期的に監視
    }
}

void AppManual::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(1);

    // PCA9685接続チェック
    if (!pca9685_connected) {
        canvas.setTextColor(RED);
        canvas.drawString("ERROR: PCA9685 NOT CONNECTED", 10, 100);
        canvas.drawString("Check I2C connection", 10, 130);
        return;
    }

    const int y_off = TOPBAR_HEIGHT;  // TopBar を避ける
    canvas.setTextSize(1);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);

    // ホームボタン（左上、2倍サイズ）
    int home_btn_x = 10;
    int home_btn_y = 8 + y_off;
    int home_btn_w = 60;
    int home_btn_h = 32;
    // 全サーボが中立にあれば色を変える
    uint16_t home_btn_color = areAllServosAtNeutral() ? GREEN : DARKGREEN;
    canvas.fillRect(home_btn_x, home_btn_y, home_btn_w, home_btn_h, home_btn_color);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);
    canvas.setTextSize(1);
    canvas.drawString("HOME", home_btn_x + home_btn_w/2, home_btn_y + home_btn_h/2);

    // タイトル
    canvas.setTextSize(1);
    canvas.drawCentreString("Servo Control (PCA9685)", SCREEN_WIDTH/2, 8 + y_off);

    // 現在選択中のサーボ表示
    char servo_info[64];
    std::sprintf(servo_info, "Servo %d: %d deg", _selected_servo, _servo_positions[_selected_servo]);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString(servo_info, 10, 50 + y_off);

    // サーボ選択ボタン（8個、上下 2 行配置、1.5倍サイズ）
    int btn_y_row1 = 70 + y_off;
    int btn_y_row2 = 120 + y_off;
    int btn_w = 40, btn_h = 45;  // 選択ボタンサイズ
    
    for (int i = 0; i < SERVO_COUNT; i++) {
        int col = i % 4;
        int row = i / 4;
        int btn_x = 0 + col * (btn_w + 5);  // 左から横並び配置
        int btn_y = (row == 0) ? btn_y_row1 : btn_y_row2;
        
        // 選択中のサーボはハイライト
        uint16_t btn_color = (i == _selected_servo) ? YELLOW : BLUE;
        canvas.fillRect(btn_x, btn_y, btn_w, btn_h, btn_color);
        canvas.setTextColor(BLACK);
        canvas.setTextSize(1);
        
        char btn_label[4];
        std::sprintf(btn_label, "S%d", i);
        canvas.setTextDatum(middle_center);
        canvas.drawString(btn_label, btn_x + btn_w/2, btn_y + btn_h/2);
    }

    // オフセット表示と調整ボタン（1.5倍に拡大し、選択ボタン群の右側に配置、上下並び）
    int offset_btn_w = 36;
    int offset_btn_h = 41;  // 縦に1.5倍（27 → 41）
    int offset_base_x = 4 * (btn_w + 5) + 10; // 選択ボタン4列ぶんの右側に少し余白を足す
    int offset_minus_x = offset_base_x;
    int offset_plus_x = offset_base_x; // 同じX座標で上下に並べる
    int offset_minus_y = 70 + y_off;
    int offset_plus_y = 70 + y_off + offset_btn_h + 5; // 少し隙間を空けて下に配置
    
    // オフセットボタン描画（上下に並べて配置）
    canvas.fillRect(offset_minus_x, offset_minus_y, offset_btn_w, offset_btn_h, DARKGREY);
    canvas.fillRect(offset_plus_x, offset_plus_y, offset_btn_w, offset_btn_h, DARKGREY);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);
    canvas.drawString("-", offset_minus_x + offset_btn_w/2, offset_minus_y + offset_btn_h/2);
    canvas.drawString("+", offset_plus_x + offset_btn_w/2, offset_plus_y + offset_btn_h/2);
    
    // Offset表示（ボタンの上）
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("Offset:", offset_minus_x, offset_minus_y - 18);
    char offset_text[32];
    std::sprintf(offset_text, "%d deg", _servo_offsets[_selected_servo]);
    canvas.drawString(offset_text, offset_minus_x, offset_minus_y - 6);

    // 角度スライダー
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("Angle:", 10, 170 + y_off);
    
    // 角度調整用の簡易スライダー表現
    int slider_y = 184 + y_off;
    int slider_w = 300;
    int slider_h = 36;  // 高さを3倍に（12 → 36）
    canvas.drawRect(10, slider_y, slider_w, slider_h, WHITE);
    
    // S4～S7は表示角度を反転
    int display_angle = _servo_positions[_selected_servo];
    if (_selected_servo >= 4) {
        display_angle = 180 - display_angle;
    }
    int fill_w = (display_angle * slider_w) / 180;

    //S0～S7の場合はfill_wを反転
    if (_selected_servo >= 4) {
        fill_w = slider_w - fill_w;
    }
    canvas.fillRect(10, slider_y, fill_w, slider_h, GREEN);

    char angle_text[32];
    std::sprintf(angle_text, "%d / 180 deg", _servo_positions[_selected_servo]);
    canvas.drawString(angle_text, 10, slider_y + slider_h + 6);
}

void AppManual::onTouch(int x, int y) {}

void AppManual::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppManual::handlePress(int16_t x, int16_t y) {
    // ホームボタン判定（左上、2倍サイズ）
    const int y_off = TOPBAR_HEIGHT;
    int home_btn_x = 10;
    int home_btn_y = 8 + y_off;
    int home_btn_w = 60;
    int home_btn_h = 32;
    
    if (x >= home_btn_x && x < home_btn_x + home_btn_w && y >= home_btn_y && y < home_btn_y + home_btn_h) {
        // すべてのサーボを中立（90度）に設定
        for (int i = 0; i < SERVO_COUNT; i++) {
            setServoAngle(i, 90);
        }
        Serial.println("All servos reset to neutral (90 degrees)");
        return;
    }

    // オフセット調整ボタン判定（1.5倍サイズ、選択ボタン右側配置、上下並び）
    const int offset_btn_w = 36;
    const int offset_btn_h = 41;  // 縦に1.5倍（27 → 41）
    const int btn_w = 40; // 描画と合わせる
    const int offset_base_x = 4 * (btn_w + 5) + 10;
    const int offset_minus_x = offset_base_x;
    const int offset_plus_x = offset_base_x; // 同じX座標
    const int offset_minus_y = 70 + y_off;
    const int offset_plus_y = 70 + y_off + offset_btn_h + 5; // 少し隙間を空けて下に配置
    constexpr int OFFSET_MIN = -45;
    constexpr int OFFSET_MAX = 45;

    // マイナスボタン（上）
    if (x >= offset_minus_x && x < offset_minus_x + offset_btn_w && y >= offset_minus_y && y < offset_minus_y + offset_btn_h) {
        _servo_offsets[_selected_servo] -= 1;
        if (_servo_offsets[_selected_servo] < OFFSET_MIN) _servo_offsets[_selected_servo] = OFFSET_MIN;
        saveOffsets();
        setServoAngle(_selected_servo, _servo_positions[_selected_servo]);
        return;
    }
    // プラスボタン（下）
    if (x >= offset_plus_x && x < offset_plus_x + offset_btn_w && y >= offset_plus_y && y < offset_plus_y + offset_btn_h) {
        _servo_offsets[_selected_servo] += 1;
        if (_servo_offsets[_selected_servo] > OFFSET_MAX) _servo_offsets[_selected_servo] = OFFSET_MAX;
        saveOffsets();
        setServoAngle(_selected_servo, _servo_positions[_selected_servo]);
        return;
    }

    // サーボ選択ボタン判定（TOPBAR_HEIGHT を考慮）
    const int btn_y_row1 = 70 + y_off;
    const int btn_y_row2 = 120 + y_off;
    const int btn_h = 45;  // 描画と合わせる
    const int slider_w = 300;


    // サーボボタンをタッチ判定
    for (int i = 0; i < SERVO_COUNT; i++) {
        int col = i % 4;
        int row = i / 4;
        int btn_x = 0 + col * (btn_w + 5);
        int btn_y = (row == 0) ? btn_y_row1 : btn_y_row2;

        if (x >= btn_x && x < btn_x + btn_w && y >= btn_y && y < btn_y + btn_h) {
            _selected_servo = i;
            Serial.printf("Selected servo %d\n", i);
            return;
        }
    }

    // 角度調整ボタン判定
    int slider_y = 184 + y_off;
    int slider_h = 36;  // 高さを3倍に（12 → 36）

    // スライダー直接タッチ
    if (y >= slider_y && y < slider_y + slider_h && x >= 10 && x < 10 + slider_w) {
        int angle = ((x - 10) * 180) / slider_w;
        setServoAngle(_selected_servo, angle);
    }
}

void AppManual::handleMove(int16_t x, int16_t y) {
    // スライダーをドラッグで角度調整
    const int y_off = TOPBAR_HEIGHT;
    int slider_y = 184 + y_off;
    int slider_h = 36;  // 高さを3倍に（12 → 36）
    int slider_w = 300;
    if (y >= slider_y && y < slider_y + slider_h && x >= 10 && x < 10 + slider_w) {
        int angle = ((x - 10) * 180) / slider_w;
        setServoAngle(_selected_servo, angle);
    }
}

void AppManual::handleRelease(int16_t x, int16_t y) {}

void AppManual::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    uint16_t bg = pressed ? DARKGREEN : GREEN;
    canvas.fillRoundRect(x, y, w, h, 10, bg);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
}
