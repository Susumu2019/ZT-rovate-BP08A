/**
 ****************************************************************************
 * @file     TopBar.h
 * @brief    TopBar ウィジェット - 画面上部のバー
 * @details  アプリ名とライフカウンターを表示
 * @version  V1.0
 * @date     2025-10-3
 * @author   Susumu Hirai
 *****************************************************************************
 */
#pragma once

#include <M5CoreS3.h>
#include <lgfx/v1/LGFX_Sprite.hpp>

/**
 * @class TopBar
 * @brief 画面上部のステータスバーを管理するクラス
 * 
 * 表示内容:
 * - 左側: 現在のアプリ名
 * - 右側: MODEとライフカウンター
 */
class TopBar {
public:
  void setWiFiConnected(bool connected);
  void begin();
  void update();
  void draw(M5Canvas& canvas);
  int height() const { return height_; }
  void setTitle(const String& t) { title_ = t; }
  int getWiFiSignalStrength() const;
  void setUdpConnected(bool connected) { udpConnected_ = connected; }
  void notifyUdpSent() { lastUdpSentMs_ = millis(); udpSentCount_++; }
  void notifySerialSent() { lastSerialSentMs_ = millis(); serialSentCount_++; }

private:
  bool wifiConnected_ = false;
  bool udpConnected_ = false;
  uint32_t lastUdpSentMs_ = 0;
  uint32_t lastSerialSentMs_ = 0;
  uint32_t udpSentCount_ = 0;
  uint32_t serialSentCount_ = 0;
  void drawWiFiIcon(M5Canvas& canvas, int x, int y, int rssi) const;
  int height_ = 24;
  char life_char_[6]= "_/|^-";
  int life_counter_ = 0;
  String title_;
  uint16_t bgColor_ = WHITE;
  uint16_t textColor_ = BLACK;
  uint8_t textSize_ = 1;
  LGFX_Sprite* sprite_ = nullptr;
};
