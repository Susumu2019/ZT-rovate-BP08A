/**
 ****************************************************************************
 * @file     AppAction.cpp
 * @brief    AppAction class definition
 * @version  V1.0
 * @date     2026-01-12
 *****************************************************************************
 */

 #include "AppAction.h"
#include <Preferences.h>

static const int TOPBAR_HEIGHT = 24;
static Preferences prefs;

AppAction::AppAction() : selectedMode(0), isRunning(false), currentStep(0), lastStepTime(0), pwm(0x40) {}

void AppAction::setup() {
    selectedMode = 0;
    isRunning = false;
    currentStep = 0;
    lastStepTime = 0;
    speedLevel = 1;
    updateStepInterval(); // 初期速度を反映
    buttonsInitialized = false;
    
    // 補正値をNVSから読み込む
    loadOffsets();
    
    // PCA9685の初期化
    if (!pwm.begin()) {
        // Serial.println("ERROR: PCA9685 not found at I2C 0x40!");
    } else {
        // Serial.println("AppAction: PCA9685 initialized successfully");
        pwm.setOscillatorFrequency(25000000);
        pwm.setPWMFreq(50);
    }
    
    // Serial.println("AppAction: Ready");
}

void AppAction::setModeData() {
    // 静的メンバーmodeDataは既に初期化リストで設定済み
    // Serial.println("AppAction: Mode data is set via initialization list");
}

int AppAction::getStepIntervalMs() const {
    // speedLevel: 1=100%, 2=80%, 3=60%, 4=40%, 5=20%
    // ベース速度を60msに設定し、速度割合に応じてインターバルを拡大
    int intervals[] = {60, 75, 100, 150, 300};  // ms
    if (speedLevel < 1) return intervals[0];
    if (speedLevel > 5) return intervals[4];
    return intervals[speedLevel - 1];
}

void AppAction::updateStepInterval() {
    // speedLevelに応じてSTEP_INTERVAL_MSを更新
    int intervals[] = {60, 75, 100, 150, 300};  // ms
    if (speedLevel < 1) {
        STEP_INTERVAL_MS = intervals[0];
    } else if (speedLevel > 5) {
        STEP_INTERVAL_MS = intervals[4];
    } else {
        STEP_INTERVAL_MS = intervals[speedLevel - 1];
    }
}

void AppAction::setServoAngle(int channel, int angle) {
    if (channel < 0 || channel >= SERVO_COUNT) return;
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // 中立補正を適用
    int adj_angle = angle + _servo_offsets[channel];
    if (adj_angle < 0) adj_angle = 0;
    if (adj_angle > 180) adj_angle = 180;
    
    // S4, S5, S6, S7は制御方向を反転
    if (channel >= 4) {
        adj_angle = 180 - adj_angle;
    }
    
    // 角度をパルス幅にマップ
    int pulse_length = SERVO_MIN_PULSE + (adj_angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
    uint16_t pwm_value = (pulse_length * 4096) / 20000;
    
    pwm.setPWM(channel, 0, pwm_value);
}

/**
 * @brief CSVファイルからmodeDataを読み込む
 * @param filename 読み込むファイルパス（例: "/mode_data.csv"）
 * 
 * CSV形式:
 * mode,step,s0,s1,s2,s3,s4,s5,s6,s7
 * 0,0,90,90,90,90,90,90,90,90
 * ...
 */
void AppAction::loadModeDataFromFile(const char* filename) {
    // TODO: SDカードやファイルシステムからCSVを読み込み
    // 例: LittleFS, SPIFFS, SDカードなど
    // Serial.printf("TODO: Load mode data from %s\n", filename);
}

/**
 * @brief modeDataをCSVファイルに保存
 * @param filename 保存先ファイルパス（例: "/mode_data.csv"）
 * 
 * CSV形式で3つのモードすべてを保存
 */
void AppAction::saveModeDataToFile(const char* filename) {
    // TODO: modeDataをCSV形式でファイルに保存
    // Serial.printf("TODO: Save mode data to %s\n", filename);
}

void AppAction::executeStep() {
    if (selectedMode < 1 || selectedMode > MODE_COUNT) return;
    
    int modeIndex = selectedMode - 1;
    
    // 現在のステップの角度を全サーボに送信
    for (int servo = 0; servo < SERVO_COUNT; servo++) {
        int angle = modeData[modeIndex][servo][currentStep];
        setServoAngle(servo, angle);
    }
    
    // 次のステップへ
    currentStep++;
    if (currentStep >= STEP_COUNT) {
        currentStep = 0; // ループ
    }
}

void AppAction::loop() {
    btnMgr.updateAll();
    
    // 実行中の場合、速度レベルに基づいたインターバルでステップを進める
    if (isRunning && selectedMode > 0) {
        unsigned long currentTime = millis();
        
        // 初回実行時の初期化
        if (lastStepTime == 0) {
            lastStepTime = currentTime;
        }
        
        // 経過時間を計算
        unsigned long elapsed = currentTime - lastStepTime;
        
        // 実行すべきステップ数を計算（小数点以下は次フレームに持ち越す）
        int stepsToExecute = (int)(elapsed / STEP_INTERVAL_MS);
        
        if (stepsToExecute > 0) {
            // 複数ステップを実行
            for (int i = 0; i < stepsToExecute; i++) {
                executeStep();
            }
            
            // lastStepTimeを更新（実行したステップ数分だけ進める）
            lastStepTime += (unsigned long)stepsToExecute * STEP_INTERVAL_MS;
        }
    }
}

void AppAction::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    
    const int y_offset = TOPBAR_HEIGHT;
    canvas.setTextColor(WHITE);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(middle_center);
    
    // ボタン初期化（一度だけ）
    if (!buttonsInitialized) {
        btnMgr.clear();
        
        // MODE ボタン（横並び、サイズを半分に）
        const int mode_btn_w = 60;
        const int mode_btn_h = 35;
        const int mode_y = y_offset + 10;
        const int mode_gap = 5;
        const int mode_start_x = (320 - (mode_btn_w * 3 + mode_gap * 2)) / 2;
        
        for (int i = 0; i < 3; i++) {
            int btn_x = mode_start_x + i * (mode_btn_w + mode_gap);
            uint16_t color = (selectedMode == i + 1) ? YELLOW : BLUE;
            uint16_t pressed_color = (selectedMode == i + 1) ? ORANGE : DARKBLUE;
            
            char label[16];
            sprintf(label, "MODE_%d", i + 1);
            
            CoreS3Buttons modeBtn(label, btn_x, mode_y, mode_btn_w, mode_btn_h, color, pressed_color, BLACK);
            int mode_idx = i + 1;
            modeBtn.setCallback([this, mode_idx]() {
                selectedMode = mode_idx;
                currentStep = 0; // ステップをリセット
                buttonsInitialized = false; // 色を反映するため再構築
                // Serial.printf("Selected MODE_%d\n", mode_idx);
            });
            btnMgr.addButton(std::move(modeBtn));
        }
        
        // Start/Stop ボタン（サイズを半分に）
        const int ctrl_btn_w = 70;
        const int ctrl_btn_h = 35;
        const int ctrl_y = y_offset + 55;
        const int ctrl_gap = 5;
        const int ctrl_start_x = (320 - (ctrl_btn_w * 2 + ctrl_gap)) / 2;
        
        // Start ボタン
        CoreS3Buttons startBtn("Start", ctrl_start_x, ctrl_y, ctrl_btn_w, ctrl_btn_h, 
                               GREEN, DARKGREEN, WHITE);
        startBtn.setCallback([this]() {
            if (selectedMode > 0) {
                isRunning = true;
                currentStep = 0;
                lastStepTime = millis();
                // Serial.printf("Started MODE_%d at speed level %d\n", selectedMode, speedLevel);
            } else {
                // Serial.println("Please select a mode first");
            }
        });
        btnMgr.addButton(std::move(startBtn));
        
        // Stop ボタン
        CoreS3Buttons stopBtn("Stop", ctrl_start_x + ctrl_btn_w + ctrl_gap, ctrl_y, 
                              ctrl_btn_w, ctrl_btn_h, RED, MAROON, WHITE);
        stopBtn.setCallback([this]() {
            isRunning = false;
            // Serial.println("Stopped");
        });
        btnMgr.addButton(std::move(stopBtn));
        
        // 速度調整ボタン（5段階）
        const int speed_btn_w = 55;
        const int speed_btn_h = 30;
        const int speed_y = y_offset + 95;
        const int speed_gap = 3;
        const int speed_start_x = (320 - (speed_btn_w * 5 + speed_gap * 4)) / 2;
        
        for (int i = 1; i <= 5; i++) {
            int btn_x = speed_start_x + (i - 1) * (speed_btn_w + speed_gap);
            uint16_t color = (speedLevel == i) ? YELLOW : CYAN;
            uint16_t pressed_color = (speedLevel == i) ? ORANGE : DARKCYAN;
            
            char label[8];
            sprintf(label, "%d", i);
            
            CoreS3Buttons speedBtn(label, btn_x, speed_y, speed_btn_w, speed_btn_h, color, pressed_color, BLACK);
            int speed_idx = i;
            speedBtn.setCallback([this, speed_idx]() {
                speedLevel = speed_idx;
                updateStepInterval();  // speedLevelに応じてSTEP_INTERVAL_MSを更新
                buttonsInitialized = false; // 色を反映するため再構築
                // Serial.printf("Speed level set to %d\n", speed_idx);
            });
            btnMgr.addButton(std::move(speedBtn));
        }
        
        buttonsInitialized = true;
    }
    
    // ステータス表示
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(top_center);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::Font2);
    
    const int speed_y = y_offset + 95;
    const int speed_btn_h = 30;
    
    char status[64];
    if (selectedMode > 0) {
        sprintf(status, "Selected: MODE_%d", selectedMode);
    } else {
        sprintf(status, "No mode selected");
    }
    canvas.drawString(status, 160, speed_y + speed_btn_h + 5);
    
    char running[64];
    if (isRunning) {
        sprintf(running, "Running: Step %d/%d", currentStep, STEP_COUNT);
    } else {
        sprintf(running, "Stopped: Step %d/%d", currentStep, STEP_COUNT);
    }
    canvas.drawString(running, 160, speed_y + speed_btn_h + 18);
    
    char speed_info[64];
    float speed_percent[] = {100.0f, 80.0f, 60.0f, 40.0f, 20.0f};
    sprintf(speed_info, "Speed: %d (%d%%)", speedLevel, (int)speed_percent[speedLevel - 1]);
    canvas.drawString(speed_info, 160, speed_y + speed_btn_h + 31);
    
    // デバッグ情報表示
    char debug_info[64];
    sprintf(debug_info, "Interval: %d ms (getStepInterval: %d ms)", STEP_INTERVAL_MS, getStepIntervalMs());
    canvas.setTextSize(0);
    canvas.drawString(debug_info, 160, speed_y + speed_btn_h + 44);
    canvas.setTextSize(1);
    
    btnMgr.drawAll(canvas);
}

void AppAction::onTouch(int x, int y) {
    // タッチイベント処理
}

void AppAction::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppAction::handlePress(int16_t x, int16_t y) {
    // ButtonManagerが自動的にハンドリング
}

void AppAction::loadOffsets() {
    prefs.begin("servo", true);
    for (int i = 0; i < SERVO_COUNT; i++) {
        char key[8];
        std::sprintf(key, "off%d", i);
        _servo_offsets[i] = prefs.getInt(key, 0);
    }
    prefs.end();
    // Serial.println("AppAction: Servo offsets loaded");
}
void AppAction::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {

}

uint16_t AppAction::iconBackgroundColor() const { return BLUE; }
uint16_t AppAction::iconPressedColor() const { return DARKBLUE; }
uint16_t AppAction::iconTextColor() const { return WHITE; }
