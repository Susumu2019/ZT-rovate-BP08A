#include "TopBar.h"
#include <M5CoreS3.h>
#include "timer/timer.h"
#include "system/system.h"
#include "UI/Icon/Icon.h"
#include <WiFi.h>  

// 外部で定義されたグローバル変数
extern PublicTimer publicTimer;  // タイマー管理
extern Icon icon;  // アイコン描画

/**
 * @brief TopBar の初期化
 */
void TopBar::begin() {
  if (!sprite_) {
    sprite_ = new LGFX_Sprite(&M5.Lcd);
    sprite_->setPsram(true);  // PSRAMを使用してメモリ効率化
  }
  sprite_->setFont(&fonts::Font2);
  sprite_->setTextSize(1);
  int fh = 16;  // フォント高さ
  height_ = fh + 4;  // 上下のマージンを含めた高さ
  sprite_->createSprite(M5.Lcd.width(), height_);
}

/**
 * @brief TopBar の毎フレーム更新処理
 * 
 * 現在は特に処理がない（将来のアニメーション用にスタブとして残してある）
 */
void TopBar::update() {
  // 静的なタイトルだけを表示するため、定期更新は不要
  // 将来的にアニメーション機能を追加する際に使用
}

/**
 * @brief TopBar を Canvas に描画
 * 
 * @param canvas 描画対象の Canvas
 * 
 * 描画要素:
 * 1. 背景色で全体を塗りつぶし
 * 2. 左側: アプリ名
 * 3. 右側: MODE値とライフカウンター
 */
void TopBar::draw(M5Canvas& canvas) {
  if (!sprite_) return;
  
  // 背景色で全体を塗りつぶし
  canvas.fillRect(0, 0, M5.Lcd.width(), height_, bgColor_);
  
  if (title_.length() > 0) {
    canvas.setTextColor(textColor_);
    canvas.setTextDatum(ML_DATUM);  // 左中央を基準
    canvas.setTextSize(textSize_);
    canvas.setFont(&fonts::Font2);
    
    // 左側: アプリ名を表示
    canvas.drawString(title_, 4, height_/2);

    // 右側: MODE値を表示
    char setLabel[64];
    sprintf(setLabel, "MODE:%d", mode); 
    canvas.drawString(setLabel, 180, height_/2);
    
    // WiFi信号強度を表示（MODE表示の左側）
    int rssi = getWiFiSignalStrength();
    drawWiFiIcon(canvas, 150, height_/2, rssi);
    // WiFi接続時は枠や色で強調
    if (wifiConnected_) {
      canvas.drawRect(142, (height_/2)-10, 16, 20, GREEN);
    }
    
    // UDP/Serial送信インジケーター（WiFiアイコンの左側）
    uint32_t now = millis();
    bool udpActive = (now - lastUdpSentMs_) < 200;  // 200ms間点灯
    bool serialActive = (now - lastSerialSentMs_) < 200;
    
    // UDP インジケーター（小さい円）
    int udpX = 130;
    int udpY = height_ / 2;
    uint16_t udpColor = udpConnected_ ? (udpActive ? BLUE : GREEN) : DARKGREY;
    canvas.fillCircle(udpX, udpY, 3, udpColor);
    canvas.drawString("U", udpX - 10, udpY);
    
    // Serial インジケーター（小さい円）
    int serialX = 110;
    int serialY = height_ / 2;
    uint16_t serialColor = serialActive ? CYAN : DARKGREY;
    canvas.fillCircle(serialX, serialY, 3, serialColor);
    canvas.drawString("S", serialX - 10, serialY);
    
    // 右側: バッテリー残量を表示
    int battery_level = M5.Power.getBatteryLevel();
    sprintf(setLabel, "%d%%", battery_level);
    canvas.drawString(setLabel, 270, height_/2);
    
    // 右側: ライフカウンター（アニメーション文字）を表示
    sprintf(setLabel, "%c", life_char_[publicTimer.getLifeCounter()]); 
    canvas.drawString(setLabel, 310, height_/2);
  }
}

/**
 * @brief WiFi電波強度を取得
 * @return RSSI値（dBm）。WiFi未接続時は-100を返す
 */
int TopBar::getWiFiSignalStrength() const {
  if (WiFi.status() != WL_CONNECTED) {
    return -100;  // 未接続
  }
  return WiFi.RSSI();
}

/**
 * @brief WiFi信号強度アイコンを描画
 * @param canvas 描画対象
 * @param x アイコン中心X座標
 * @param y アイコン中心Y座標
 * @param rssi 信号強度（dBm）
 * 
 * RSSI範囲とバー表示:
 * - rssi >= -50: 強い（3本）
 * - rssi >= -70: 中程度（2本）
 * - rssi >= -85: 弱い（1本）
 * - rssi < -85: 非常に弱い/未接続（×印）
 */
void TopBar::drawWiFiIcon(M5Canvas& canvas, int x, int y, int rssi) const {
  const int barWidth = 2;
  const int barSpacing = 2;
  const int baseHeight = 4;
  
  // 信号強度からバー数を決定
  int barCount = 0;
  if (rssi >= -50) barCount = 3;      // 強い
  else if (rssi >= -70) barCount = 2; // 中程度
  else if (rssi >= -85) barCount = 1; // 弱い
  else barCount = 0;                   // 未接続
  
  // 色を決定
  uint16_t color = (barCount >= 2) ? GREEN : (barCount == 1) ? ORANGE : RED;
  
  if (barCount == 0) {
    // 未接続：×印を表示
    canvas.drawLine(x-3, y-3, x+3, y+3, RED);
    canvas.drawLine(x-3, y+3, x+3, y-3, RED);
  } else {
    // 信号バーを描画（左から右に高くなる）
    for (int i = 0; i < 3; i++) {
      int barHeight = baseHeight + (i * 3);
      int barX = x - 6 + (i * (barWidth + barSpacing));
      int barY = y - barHeight / 2;
      
      // アクティブなバーのみ塗りつぶし
      if (i < barCount) {
        canvas.fillRect(barX, barY, barWidth, barHeight, color);
      } else {
        canvas.drawRect(barX, barY, barWidth, barHeight, DARKGREY);
      }
    }
  }
}

void TopBar::setWiFiConnected(bool connected) {
  wifiConnected_ = connected;
}

/*********************************** END OF FILE ******************************/