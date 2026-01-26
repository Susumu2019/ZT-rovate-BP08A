#include "system/Settings.h"
#include <M5CoreS3.h>
#include <WiFi.h>
#include "config.h"

#include "UI/TopBar/TopBar.h"
#include "App/AppManager/AppManager.h"
extern AppManager appManager;
#define TOPBAR_HEIGHT 24
#include <cstring>

// 画面幅に収まるようにテキストを省略（最大len文字、超えたら...）
void trimText(char* dst, const char* src, size_t maxLen) {
    size_t len = strlen(src);
    if (len <= maxLen) {
        strcpy(dst, src);
        return;
    }
    if (maxLen < 4) {
        strncpy(dst, src, maxLen);
        dst[maxLen] = '\0';
        return;
    }
    strncpy(dst, src, maxLen - 3);
    strcpy(dst + maxLen - 3, "...");
}

// 未対応文字を?に置換（半角英数字と一部記号のみ許可）
void sanitizeText(char* dst, const char* src, size_t maxLen) {
    size_t i;
    for (i = 0; i < maxLen && src[i]; ++i) {
        char c = src[i];
        if ((c >= 32 && c <= 126)) {
            dst[i] = c;
        } else {
            dst[i] = '?';
        }
    }
    dst[i] = '\0';
}
#include "AppWifi.h"
#include "config.h"
#include "system/system.h"
#include "UI/Icon/Icon.h"
#include "timer/timer.h"

#include "system/comm/UdpSender.h"

static UdpSender udpSender;
static bool pcConnected = false;
static uint32_t lastSignalMs = 0;

AppWifi::AppWifi() {}

void AppWifi::setup() {
    mode = 200; // AppWifi mode
}

void AppWifi::loop() {
    // UDP接続状態はudpSender.isReady()で判定
    // PCからのUDP受信は本来UdpSenderで管理するが、ここでは状態のみ表示
    // 受信検知は必要ならUdpSenderに拡張可
    // ここでは5秒以内に送信があれば接続済みとみなす（仮実装）
    if (udpSender.isReady()) {
        if (millis() - lastSignalMs < 5000) {
            pcConnected = true;
        } else {
            pcConnected = false;
        }
    } else {
        pcConnected = false;
    }
}

void AppWifi::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    canvas.setTextSize(1);
    char setLabel[64];
    char safeText[40];
    char trimmed[40];
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);
    // SSID
    sanitizeText(safeText, WIFI_SSID, 32);
    trimText(trimmed, safeText, 18); // 18文字程度で省略
    sprintf(setLabel, "WiFi SSID: %s", trimmed);
    canvas.drawString(setLabel, 160, 60 + TOPBAR_HEIGHT - 50);
    canvas.setFont(&fonts::Font4);
    trimText(trimmed, safeText, 12); // 大きいフォントはさらに短く
    canvas.drawString(trimmed, 160, 80 + TOPBAR_HEIGHT - 50);
    canvas.setFont(&fonts::Font2);

    // --- SSID scan result display ---
    static int lastScanResult = -2; // -2:未スキャン, -1:スキャン中, 0:見つからず, 1:見つかった
    static uint32_t lastScanTime = 0;
    static bool scanRequested = false;
    // WiFi機能ONかつ未接続時のみスキャン
    if (Settings::getInstance().isWifiEnabled() && WiFi.status() != WL_CONNECTED) {
        if (!scanRequested || millis() - lastScanTime > 5000) {
            lastScanResult = -1; // スキャン中
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            int n = WiFi.scanNetworks();
            lastScanResult = 0;
            for (int i = 0; i < n; ++i) {
                if (WiFi.SSID(i) == String(WIFI_SSID)) {
                    lastScanResult = 1;
                    break;
                }
            }
            lastScanTime = millis();
            scanRequested = true;
        }
        // 結果表示
        if (lastScanResult == -1) {
            canvas.setTextColor(YELLOW);
            canvas.drawString("Scanning SSID...", 160, 100 + TOPBAR_HEIGHT - 50);
        } else if (lastScanResult == 1) {
            canvas.setTextColor(GREEN);
            canvas.drawString("SSID found", 160, 100 + TOPBAR_HEIGHT - 50);
        } else if (lastScanResult == 0) {
            canvas.setTextColor(RED);
            canvas.drawString("SSID not found", 160, 100 + TOPBAR_HEIGHT - 50);
        }
        canvas.setTextColor(WHITE);
    } else {
        scanRequested = false;
        lastScanResult = -2;
    }
    // IP
    if (WiFi.status() == WL_CONNECTED) {
        String ipstr = WiFi.localIP().toString();
        if (ipstr == "0.0.0.0" || ipstr.length() < 7) {
            sprintf(setLabel, "IP: 未取得");
        } else {
            sanitizeText(safeText, ipstr.c_str(), 32);
            trimText(trimmed, safeText, 18);
            sprintf(setLabel, "IP: %s", trimmed);
        }
        // TopBarにWiFi接続状態を通知
        appManager.getTopBar().setWiFiConnected(true);
    } else {
        sprintf(setLabel, "IP: (未接続)");
        appManager.getTopBar().setWiFiConnected(false);
    }
    canvas.drawString(setLabel, 160, 110 + TOPBAR_HEIGHT - 50);
    // UDPポート
    sprintf(setLabel, "UDP Port: %u", UDP_TARGET_PORT);
    canvas.drawString(setLabel, 160, 130 + TOPBAR_HEIGHT - 50);
    // PC接続案内（短く分割）
    canvas.drawString("Connect from PC ->", 100, 155 + TOPBAR_HEIGHT - 50);
    if (udpSender.isReady()) {
        String ipstr = WiFi.localIP().toString();
        sanitizeText(safeText, ipstr.c_str(), 32);
        trimText(trimmed, safeText, 15);
        sprintf(setLabel, "%s:%u", trimmed, UDP_TARGET_PORT);
        canvas.drawString(setLabel, 200, 155 + TOPBAR_HEIGHT - 50);
    } else {
        canvas.drawString("Not connected", 220, 155 + TOPBAR_HEIGHT - 50);
    }
    // 接続確認シグナル
    if (pcConnected) {
        canvas.setFont(&fonts::Font4);
        canvas.setTextColor(GREEN);
        canvas.drawString("PC Connected!", 180, 210 + TOPBAR_HEIGHT - 50);
        canvas.setTextColor(WHITE);
        canvas.setFont(&fonts::Font2);
    }
    // 操作案内
    if (!Settings::getInstance().isWifiEnabled()) {
        // WiFi function is OFF
        canvas.setTextColor(RED);
        canvas.setTextSize(2);
        canvas.drawCentreString("WiFi is OFF", 160, BTN_Y + BTN_H/2 - 50);
        canvas.setTextSize(1);
        canvas.setTextColor(WHITE);
        canvas.drawString("Enable WiFi in settings", 160, BTN_Y + BTN_H/2 + 24 - 50);
    } else if (WiFi.status() != WL_CONNECTED) {
        // Show connect button only when not connected
        canvas.fillRoundRect(BTN_X, BTN_Y - 50, BTN_W, BTN_H, 10, DARKCYAN);
        canvas.setTextColor(WHITE);
        canvas.setTextSize(2);
        canvas.drawCentreString("Connect WiFi", BTN_X + BTN_W/2, BTN_Y + BTN_H/2 - 8 - 50);
        canvas.setTextSize(1);
        canvas.setTextColor(WHITE);
        canvas.drawString("WiFi not connected", 160, BTN_Y - 20 - 50);
    } else {
        canvas.drawString("Touch to reconnect/send", 160, 180 + TOPBAR_HEIGHT - 50);
        //icon.drawArrow(160, 200 + TOPBAR_HEIGHT - 50, 35, publicTimer.getFlicker500() ? WHITE : BLACK, "down");
    }
}

void AppWifi::onTouch(int x, int y) {
    // WiFi機能OFF時は何もしない
    if (!Settings::getInstance().isWifiEnabled()) {
        return;
    }
    // WiFi未接続時はボタン領域のみ有効
    if (WiFi.status() != WL_CONNECTED) {
        if (x >= BTN_X && x < BTN_X + BTN_W && y >= BTN_Y && y < BTN_Y + BTN_H) {
            udpSender.begin();
        }
    } else {
        // 接続済み時は従来通りUDP送信
        uint16_t dummyServo[8] = {0};
        udpSender.sendControl(0,0,0,0,0,0,0, dummyServo, dummyServo, 0, false);
        lastSignalMs = millis();
    }
}

void AppWifi::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppWifi::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    uint16_t bg = pressed ? DARKCYAN : CYAN;
    canvas.fillRoundRect(x, y, w, h, 10, bg);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
}

uint16_t AppWifi::iconBackgroundColor() const { return DARKBLUE; }
uint16_t AppWifi::iconPressedColor() const { return DARKGREY; }
uint16_t AppWifi::iconTextColor() const { return WHITE; }

void AppWifi::printNetworkInfo() const {
    Serial.printf("  SSID: %s\r\n", WIFI_SSID);
    if (WiFi.status() == WL_CONNECTED) {
        String ipstr = WiFi.localIP().toString();
        Serial.printf("  IP: %s\r\n", ipstr.c_str());
    } else {
        Serial.printf("  IP: (未接続)\r\n");
    }
    Serial.printf("  UDP Port: %u\r\n", UDP_TARGET_PORT);
}
