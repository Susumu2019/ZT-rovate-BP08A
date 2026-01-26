#include "AppSetup.h"
#include "config.h"
#include "system/system.h"
#include "system/Settings.h"
#include "UI/Icon/Icon.h"

#include <ArduinoJson.h>
#include "system/comm/CommProtocol.h"

// ボーレート選択肢
static const uint32_t BAUD_OPTIONS[] = {115200, 230400, 460800, 921600, 1000000, 2000000};
static const int BAUD_COUNT = 6;

AppSetup::AppSetup()
    : _serialModeToggle(10, 50, 90, 24, false, BLUE, ORANGE, WHITE, WHITE, "Text", "Binary"),
      _wifiToggle(110, 50, 60, 24, true, GREEN, RED, WHITE, WHITE, "ON", "OFF"),
      _serialToggle(180, 50, 60, 24, true, GREEN, RED, WHITE, WHITE, "ON", "OFF"),
      _imuOutputToggle(250, 50, 60, 24, true, GREEN, RED, WHITE, WHITE, "ON", "OFF"),
      _selectedBaudIndex(3) {}

void AppSetup::setup() {
    btnMgr.clear();
    mode = 900;  // AppSetup mode

    // 履歴クリア
    uart_rx_history_len = 0;
    for (int i = 0; i < RX_HISTORY_SIZE; i++) uart_rx_history[i] = 0;

    loadSettings();
    
    // トグルスイッチのコールバック
    _serialModeToggle.setCallback([this](bool v) {
        Settings::getInstance().setSerialMode(v ? Settings::SERIAL_TEXT : Settings::SERIAL_BINARY);
        Serial.printf("Serial Mode: %s\n", v ? "Text" : "Binary");
    });
    
    _wifiToggle.setCallback([this](bool v) {
        Settings::getInstance().setWifiEnabled(v);
        Serial.printf("WiFi: %s\n", v ? "ON" : "OFF");
    });
    
    _serialToggle.setCallback([this](bool v) {
        Settings::getInstance().setSerialEnabled(v);
        Serial.printf("Serial: %s\n", v ? "ON" : "OFF");
    });
    
    _imuOutputToggle.setCallback([this](bool v) {
        Settings::getInstance().setImuOutputEnabled(v);
        Serial.printf("IMU Output: %s\n", v ? "ON" : "OFF");
    });
    
    // 保存ボタン
    {
        CoreS3Buttons btnSave("Save", 200, 100, 80, 40, GREEN, DARKGREY, WHITE);
        btnSave.setOnRelease([this]() {
            saveSettings();
        });
        btnMgr.addButton(std::move(btnSave));
    }
    // PCへPingボタン
    {
        CoreS3Buttons btnPingPC("Ping PC", 200, 140, 80, 40, BLUE, DARKGREY, WHITE);
        btnPingPC.setOnRelease([this]() {
            // バイナリ/テキストモード判定
            auto& settings = Settings::getInstance();
            if (settings.getSerialMode() == Settings::SERIAL_BINARY) {
                // バイナリpingコマンド送信
                uint8_t frame[12];
                frame[0] = 0xAA; frame[1] = 0x55; // SYNC
                frame[2] = 0x01; // VER
                frame[3] = 0x02; // TYPE_COMMAND
                frame[4] = 0x00; frame[5] = 0x00; // SEQ=0
                frame[6] = 0x01; frame[7] = 0x00; // LEN=1
                frame[8] = 0x04; // CMD=PING
                uint16_t crc = CommProtocol::crc16_ccitt(&frame[2], 7);
                frame[9] = (uint8_t)(crc & 0xFF);
                frame[10] = (uint8_t)((crc >> 8) & 0xFF);
                frame[11] = 0x7E; // ETX
                Serial2.write(frame, 12);
                Serial.println("Ping PC (binary) sent");
            } else {
                // テキストpingコマンド送信
                Serial2.println("{\"cmd\":\"ping\"}");
                Serial.println("Ping PC (text) sent");
            }
        });
        btnMgr.addButton(std::move(btnPingPC));
    }
}

void AppSetup::loop() {
    _serialModeToggle.update();
    _wifiToggle.update();
    _serialToggle.update();
    _imuOutputToggle.update();
    btnMgr.updateAll();

    // UART受信履歴の更新（Serial2から取得、最大RX_HISTORY_SIZEバイト）
    static String rxBuffer = "";
    while (Serial2.available()) {
        uint8_t c = Serial2.read();
        // 履歴保存
        if (uart_rx_history_len < RX_HISTORY_SIZE) {
            uart_rx_history[uart_rx_history_len++] = c;
        } else {
            for (int i = 1; i < RX_HISTORY_SIZE; i++) {
                uart_rx_history[i - 1] = uart_rx_history[i];
            }
            uart_rx_history[RX_HISTORY_SIZE - 1] = c;
        }
        // --- PONG応答検出（AA 55 01 02 ... 04 ... 7E）---
        if (uart_rx_history_len >= 12) {
            int i = uart_rx_history_len - 12;
            if (uart_rx_history[i] == 0xAA && uart_rx_history[i+1] == 0x55 && uart_rx_history[i+2] == 0x01 && uart_rx_history[i+3] == 0x02 && uart_rx_history[i+8] == 0x04 && uart_rx_history[i+11] == 0x7E) {
                ping_ok = true;
                ping_ok_time = millis();
            }
        }
        // --- テキスト/JSONコマンド応答 ---
        if (c == '\n' || c == '\r') {
            if (rxBuffer.length() > 0) {
                // "p"コマンド
                if (rxBuffer == "p") {
                    Serial2.println("ping ok.");
                    Serial.println("AppSetup: single 'p' received, pong sent");
                    rxBuffer = "";
                    continue;
                }
                // "?"コマンド
                if (rxBuffer == "?") {
                    Serial2.println("[コマンド一覧]");
                    Serial2.println("p         : ping応答 (通信確認)\r\n 例: p");
                    Serial2.println("{\"cmd\":\"ping\"} : ping応答(JSON)\r\n 例: {\\\"cmd\\\":\\\"ping\\\"}");
                    Serial2.println("{\"cmd\":\"servo\",\"pos\":[90,90,...]} : サーボ一括制御（角度0～180,中立90）\r\n 例: {\\\"cmd\\\":\\\"servo\\\",\\\"pos\\\":[90,90,90,90,90,90,90,90]}");
                    Serial2.println("{\"cmd\":\"offset\",\"off\":[0,0,...]} : サーボオフセット一括設定\r\n 例: {\\\"cmd\\\":\\\"offset\\\",\\\"off\\\":[0,0,0,0,0,0,0,0]}");
                    Serial2.println("{\"cmd\":\"set\",\"id\":0,\"val\":90} : 単一サーボ制御（角度0～180,中立90）\r\n 例: {\\\"cmd\\\":\\\"set\\\",\\\"id\\\":0,\\\"val\\\":90}");
                    Serial2.println("{\"cmd\":\"reset\"} : サーボ全リセット\r\n 例: {\\\"cmd\\\":\\\"reset\\\"}");
                    Serial2.println("?         : この説明を表示\r\n 例: ?");
                    Serial.println("AppSetup: '?' received, help sent");
                    rxBuffer = "";
                    continue;
                }
                // JSONコマンド
                if (rxBuffer.startsWith("{")) {
                    #include <ArduinoJson.h>
                    StaticJsonDocument<128> doc;
                    DeserializationError error = deserializeJson(doc, rxBuffer);
                    if (!error) {
                        String cmd = doc["cmd"].as<String>();
                        if (cmd == "ping") {
                            StaticJsonDocument<64> resp;
                            resp["resp"] = "pong";
                            resp["millis"] = millis();
                            serializeJson(resp, Serial2);
                            Serial2.println();
                            Serial.println("AppSetup: ping received, pong sent");
                        }
                    }
                    rxBuffer = "";
                    continue;
                }
                rxBuffer = "";
            }
        } else {
            if (rxBuffer.length() < 128) rxBuffer += (char)c;
        }
    }
    // Ping OK表示は2秒で消す
    if (ping_ok && millis() - ping_ok_time > 2000) {
        ping_ok = false;
    }
}

void AppSetup::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_left);
    
    const int y_off = 28;  // TopBar height
    
    // 横並びラベル：シリアルモード、WiFi、Serial、IMU
    canvas.drawString("Mode:", 10, 40);
    canvas.drawString("WiFi:", 110, 40);
    canvas.drawString("Serial:", 180, 40);
    canvas.drawString("IMU:", 250, 40);
    
    // トグルスイッチ（横並び）
    _serialModeToggle.draw(canvas);
    _wifiToggle.draw(canvas);
    _serialToggle.draw(canvas);
    _imuOutputToggle.draw(canvas);
    
    // ボーレート選択
    canvas.drawString("Baud Rate:", 10, 90);
    drawBaudSelector(canvas, 10, 105);
    
    // ボタン
    btnMgr.drawAll(canvas);
    // Ping OK表示
    if (ping_ok) {
        canvas.setTextColor(GREEN);
        canvas.drawString("Ping OK!", 200, 190);
    }
    // --- UART受信履歴表示 ---
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(1);
    canvas.setTextColor(CYAN);
    int rx_y = 170;
    canvas.drawString("UART RX History (Hex):", 10, rx_y);
    char hexbuf[3 * RX_HISTORY_SIZE + 1] = {};
    int idx = 0;
    for (int i = 0; i < uart_rx_history_len; i++) {
        snprintf(hexbuf + idx, 4, "%02X ", uart_rx_history[i]);
        idx += 3;
    }
    canvas.setTextColor(WHITE);
    canvas.drawString(hexbuf, 10, rx_y + 18);
}

void AppSetup::onTouch(int x, int y) {}

void AppSetup::handleTouch(int16_t x, int16_t y) {}

void AppSetup::handlePress(int16_t x, int16_t y) {
    handleBaudTouch(x, y);
}

void AppSetup::handleMove(int16_t x, int16_t y) {}

void AppSetup::handleRelease(int16_t x, int16_t y) {}

void AppSetup::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    uint16_t bg = pressed ? iconPressedColor() : iconBackgroundColor();
    canvas.fillRoundRect(x, y, w, h, 8, bg);
    canvas.setTextColor(iconTextColor());
    canvas.setTextDatum(middle_center);
    canvas.setTextSize(1);
    canvas.drawString("SET", x + w/2, y + h/2);
}

void AppSetup::loadSettings() {
    auto& settings = Settings::getInstance();
    _serialModeToggle.setValue(settings.getSerialMode() == Settings::SERIAL_TEXT);
    _wifiToggle.setValue(settings.isWifiEnabled());
    _serialToggle.setValue(settings.isSerialEnabled());
    _imuOutputToggle.setValue(settings.isImuOutputEnabled());
    
    // ボーレートインデックスを検索
    uint32_t currentBaud = settings.getSerialBaud();
    for (int i = 0; i < BAUD_COUNT; i++) {
        if (BAUD_OPTIONS[i] == currentBaud) {
            _selectedBaudIndex = i;
            break;
        }
    }
}

void AppSetup::saveSettings() {
    Settings::getInstance().setSerialBaud(BAUD_OPTIONS[_selectedBaudIndex]);
    Settings::getInstance().save();
    
    // 保存完了メッセージ
    M5.Speaker.tone(2000, 100);
    Serial.println("Settings saved!");
}

void AppSetup::drawBaudSelector(M5Canvas &canvas, int x, int y) {
    const int boxW = 50;
    const int boxH = 24;
    const int spacing = 2;
    
    for (int i = 0; i < BAUD_COUNT; i++) {
        int posX = x + (i % 3) * (boxW + spacing);
        int posY = y + (i / 3) * (boxH + spacing);
        
        uint16_t bgColor = (i == _selectedBaudIndex) ? CYAN : DARKGREY;
        uint16_t textColor = (i == _selectedBaudIndex) ? BLACK : WHITE;
        
        canvas.fillRoundRect(posX, posY, boxW, boxH, 4, bgColor);
        canvas.setTextColor(textColor);
        canvas.setTextDatum(middle_center);
        canvas.setTextSize(1);
        
        // ボーレート表示（単位省略）
        char label[16];
        if (BAUD_OPTIONS[i] >= 1000000) {
            snprintf(label, sizeof(label), "%.1fM", BAUD_OPTIONS[i] / 1000000.0f);
        } else {
            snprintf(label, sizeof(label), "%luK", (unsigned long)(BAUD_OPTIONS[i] / 1000));
        }
        canvas.drawString(label, posX + boxW/2, posY + boxH/2);
    }
}

void AppSetup::handleBaudTouch(int16_t x, int16_t y) {
    const int baseX = 10;
    const int baseY = 105;  // drawBaudSelectorと同じY座標
    const int boxW = 50;
    const int boxH = 24;
    const int spacing = 2;
    
    for (int i = 0; i < BAUD_COUNT; i++) {
        int posX = baseX + (i % 3) * (boxW + spacing);
        int posY = baseY + (i / 3) * (boxH + spacing);
        
        if (x >= posX && x <= posX + boxW && y >= posY && y <= posY + boxH) {
            _selectedBaudIndex = i;
            Serial.printf("Baud selected: %lu\n", (unsigned long)BAUD_OPTIONS[i]);
            break;
        }
    }
}