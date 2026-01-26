/**
 ****************************************************************************
 * @file     timer.cpp
 * @brief    timer for M5Stack CoreS3 SE Source File
 * @version  V1.0
 * @date     2025-10-1
 * @author   Atsuya Kusui
 *****************************************************************************
 */
#include "timer.h"

PublicTimer publicTimer;
// static instance 初期化
PublicTimer* PublicTimer::instance_ = nullptr;

PublicTimer::PublicTimer() {
  // singleton
  instance_ = this;
}

PublicTimer::~PublicTimer() {
  end();
  if (instance_ == this) instance_ = nullptr;
}

/**
 *  @brief タイマー初期化
*/
void PublicTimer::begin() {
  timer_ = timerBegin(0, 80, true);
  // static 関数を渡す（メンバではなく普通の関数ポインタとして渡せる）
  timerAttachInterrupt(timer_, &PublicTimer::onPublicTimer, true);
  timerAlarmWrite(timer_, 10000, true);
  timerAlarmEnable(timer_);
}

void PublicTimer::end() {
  if (timer_) {
    timerAlarmDisable(timer_);
    timerDetachInterrupt(timer_);
    timerEnd(timer_);
    timer_ = nullptr;
  }
}

/**
 *  @brief タイマー割り込み処理
*/
void IRAM_ATTR PublicTimer::onPublicTimer() {
  if (PublicTimer::instance_) {
    PublicTimer::instance_->handleISR();
  }
}

// 実際の ISR 処理（インスタンスメソッド）
void IRAM_ATTR PublicTimer::handleISR() {
    portENTER_CRITICAL(&mux_);

    // 10ms カウンタ（1..4）
    count_timer_10_one_ = 1;
    if (count_timer_10_ == 4) {
        count_timer_10_ = 1; // 4の次は1に戻す
    } else {
        ++count_timer_10_;
    }

    // 100ms カウンタ（0..9）
    if (count_timer_100_ == 9) {
        count_timer_100_      = 0;
        count_timer_100_one_  = 1;
        count_timer_LCD_one_  = 1;
        count_flicker_100_    = (count_flicker_100_ == 0) ? 1 : 0;
        if(life_counter_ < 4){
          life_counter_++;
        } else {
          life_counter_ = 0; 
        }
    } else {
        ++count_timer_100_;
    }

    // 500ms カウンタ（0..49）
    if (count_timer_500_ == 49) {
        count_timer_500_      = 0;
        count_timer_500_one_  = 1;
        count_flicker_500_    = (count_flicker_500_ == 0) ? 1 : 0;
    } else {
        ++count_timer_500_;
    }

    // 1000ms カウンタ（0..99）
    if (count_timer_1000_ == 99) {
        count_timer_1000_       = 0;
        count_timer_1000_one_   = 1;
        count_flicker_1000_     = (count_flicker_1000_ == 0) ? 1 : 0;
    } else {
        ++count_timer_1000_;
    }

    portEXIT_CRITICAL(&mux_);
}