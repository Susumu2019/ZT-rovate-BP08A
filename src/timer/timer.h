/**
 ****************************************************************************
 * @file     timer.h
 * @brief    timer for M5Stack CoreS3 SE Header File
 * @version  V1.0
 * @date     2025-10-1
 * @author   Atsuya Kusui
 *****************************************************************************
 */
#pragma once
#include <M5CoreS3.h>

class PublicTimer {
public:
    PublicTimer();
    ~PublicTimer();

    // Initialize timer. alarm_us: alarm period in microseconds (default 10000 = 10ms)
    void begin();
    void end();

    // ISR に登録する関数（static wrapper）
    static void IRAM_ATTR onPublicTimer();

    // Singleton accessor (returns the instance registered in ctor)
    static PublicTimer& instance();
 
private:
    void IRAM_ATTR handleISR(); // 実際のインスタンス処理（ISR 呼び出し元から呼ぶ）
    static PublicTimer* instance_; // singleton ポインタ (set in ctor)

    // hardware timer handle
    hw_timer_t* timer_ = nullptr;
    volatile int8_t   count_timer_10_ = 0;             /*10msカウンタ*/   
    volatile int8_t   count_timer_10_one_ = 0;         /*10msカウンタ（1回のみ）*/
    volatile int8_t   count_timer_100_ = 0;            /*100msカウンタ*/
    volatile int8_t   count_timer_100_one_ = 0;        /*100msカウンタ（1回のみ）*/
    volatile uint8_t  count_timer_500_ = 0;            /*500msカウンタ*/
    volatile uint8_t  count_timer_500_one_ = 0;        /*500msカウンタ（1回のみ）*/
    volatile int16_t  count_timer_1000_ = 0;           /*1000msカウンタ*/
    volatile int16_t  count_timer_1000_one_ = 0;       /*1000msカウンタ（1回のみ）*/
    volatile int16_t  count_timer_LCD_one_ = 0;        /*LCD表示更新用タイマー割り込みカウンタ（1回のみ）*/
    volatile uint16_t count_flicker_100_ = 0;          /*100ms毎の点滅カウンタ*/
    volatile uint16_t count_flicker_500_ = 0;          /*500ms毎の点滅カウンタ*/
    volatile int16_t  count_flicker_1000_ = 0;         /*1000ms毎の点滅カウンタ*/
    volatile int life_counter_ = 0;               /*フリーズ確認カウンタ*/

    volatile SemaphoreHandle_t timerSemaphore_ = NULL;
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;

    // 禁止: コピー/ムーブ
    PublicTimer(const PublicTimer&) = delete;
    PublicTimer& operator=(const PublicTimer&) = delete;
    PublicTimer(PublicTimer&&) = delete;
    PublicTimer& operator=(PublicTimer&&) = delete;

public:
    int8_t getCount10() const { return count_timer_10_; }
    int8_t getCount10One() const { return count_timer_10_one_; }
    int8_t getCount100() const { return count_timer_100_; }
    int8_t getCount100One() const { return count_timer_100_one_; }
    uint8_t getCount500() const { return count_timer_500_; }
    uint8_t getCount500One() const { return count_timer_500_one_; }
    int16_t getCount1000() const { return count_timer_1000_; }
    int16_t getCount1000One() const { return count_timer_1000_one_; }
    int16_t getCountLCDOne() const { return count_timer_LCD_one_; }
    uint16_t getFlicker100() const { return count_flicker_100_; }
    uint16_t getFlicker500() const { return count_flicker_500_; }
    int16_t getFlicker1000() const { return count_flicker_1000_; }
    int getLifeCounter() const { return life_counter_; }
};

extern PublicTimer publicTimer;
/*********************************** END OF FILE ******************************/