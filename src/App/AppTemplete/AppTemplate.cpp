/**
 ****************************************************************************
 * @file     AppTemplate.cpp
 * @brief    AppTemplate class definition
 * @note     アプリのソースファイルのテンプレートです。新しいアプリを作成する際の基礎として使用してください。
 * @version  V1.0
 * @date     2025-10-3
 * @author   Susumu Hirai
 *****************************************************************************
 */
#include "AppTemplate.h"

AppTemplate::AppTemplate() {}

void AppTemplate::setup() {
    // アプリの初期化コードをここに記述
}

void AppTemplate::loop() {
    // アプリのループ処理
}

void AppTemplate::draw(M5Canvas &canvas) {
    // アプリの画面描画コードをここに記述
}

void AppTemplate::onTouch(int x, int y) {
    // タッチイベント処理
}

void AppTemplate::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppTemplate::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {

}

uint16_t AppTemplate::iconBackgroundColor() const { return GREEN; }
uint16_t AppTemplate::iconPressedColor() const { return DARKGREEN; }
uint16_t AppTemplate::iconTextColor() const { return WHITE; }

