#include "AppMotor.h"
#include "config.h"
#include "system/system.h"
#include "UI/Icon/Icon.h"
#include "timer/timer.h"
#include <cstdio>
#include "UI/Button/Button.h"

/**
 * @file AppMotor.cpp
 * @brief モーター制御アプリの実装
 *
 * ここでは主に以下を提供する:
 * - UI 要素（スライダー、トグル、送信ボタン）の初期化
 * - `mode` による状態遷移（主モード: 400 系、サブモード: 410 系など）
 * - ハードウェア制御のための MotorControl への値反映
 */

MotorControl motorControl;

AppMotor::AppMotor()
    : _freqSlider(40, 140, 240, 12, 440), _dir_toggle(40, 180, 100, 28, true, BLUE, RED, WHITE, WHITE, "FWD", "REV") {}
     

/**
 * @brief アプリ初期化
 *
 * - ボタンマネージャのクリア
 * - `mode` を AppMotor の主モード（400）に設定
 * - 各 UI 要素のコールバック登録
 *
 * mode の扱いについて:
 * - 400: 待機／初期化完了
 * - 401: セルフチェック実行中
 * - 410-419: ユーザーがタッチ操作でデバイスを選択・制御している状態
 */
void AppMotor::setup() {
    // アプリモーターの初期化
    btnMgr.clear();
    Serial.println("AppMotor: setup called");
    
    // AppMotor の主モードを設定（サブモードは loop()/イベントで変更する）
    mode = 400; // 待機状態
    
    // トグルとスライダーの基本コールバックを設定（ログ出力等）
    _dir_toggle.setCallback([](bool v){});
    _freqSlider.setOnChange([](int v){});
    // 周波数スライダーのレンジ設定（例）
    _freqSlider.setRange(20, 2000);
    _freqSlider.setOnChange([this](int v){
        // スライダー変更時: 内部状態を更新してログ出力
        _current_frequency = v;
        Serial.printf("Frequency set to: %d\n", _current_frequency);
    });
    _dir_toggle.setCallback([this](bool v){
        _direction = v ? 1 : -1;
        Serial.printf("Direction set to: %s\n", v ? "Forward" : "Backward");
    });

    // Send Pulse ボタンの作成とイベント登録
    {
        int btnB_x = 240, btnB_y = 180, btnB_w = 80, btnB_h = 60;
        CoreS3Buttons btnSendPulse("Send Pulse", btnB_x, btnB_y, btnB_w, btnB_h, BLUE, DARKGREY, WHITE);
        // 押している間は送信フラグを立てる
        btnSendPulse.setCallback([this]() {
            _is_sending_pulse = true;
        }, EVENT_TYPE::PRESSING);
        // 離したら送信停止
        btnSendPulse.setOnRelease([this]() {
            _is_sending_pulse = false;
        });
        btnMgr.addButton(std::move(btnSendPulse));
    }
    
}

/**
 * @brief メインループ処理
 *
 * `mode` の値域で振る舞いを分岐します。
 * 例:
 * - mode >= 400 && mode < 410 : 初期化／セルフチェック／待機など
 * - mode >= 410 && mode < 420 : デバイス選択や詳細制御用の UI 表示/入力処理
 *
 * 実装方針:
 * - タッチやボタンイベントが発生したら `mode` の値を変更して状態遷移させる。
 * - 描画処理 (`draw`) は `mode` に応じて表示内容を切り替える。
 */
void AppMotor::loop() {
    // モード値の範囲でアプリの動作を分ける
    if (mode >= 400 && mode < 410) {
        // 初期化、セルフチェックなど


        mode = 410; // 特に無いので次へ
    } else if (mode >= 410 && mode < 420) {
        // デバイスの選択等（タッチでモーターを選び、選択後に個別制御モードへ）
        // 例: mode = 411 (モーター1選択), mode = 412 (モーター2選択)
    }
    
    // UI ウィジェットの更新
    _dir_toggle.update();
    _freqSlider.update();

    // 現在の内部状態を MotorControl に反映
    Serial.printf("motorControl.getDirection(): %d, motorControl.isSendingPulse(): %s\n", motorControl.getDirection(), motorControl.isSendingPulse() ? "true" : "false");
    motorControl.setSendingPulse(_is_sending_pulse);
    motorControl.setDirection(_direction);
    motorControl.setCurrentFrequency(_current_frequency);
}

/**
 * @brief 画面描画
 *
 * - UI 要素（トグル、スライダー、各ボタン）を描画
 * - `mode` に応じて必要な情報（選択中のモーター、ステータス等）を追加描画する
 */
void AppMotor::draw(M5Canvas &canvas) {
    // 画面全体の初期化
    canvas.fillScreen(BLACK);
    canvas.setTextSize(1);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(WHITE);
    canvas.setTextDatum(middle_center);

    // TopBar の高さ分だけ下に表示するためのオフセット
    const int y_off = 24;

    // mode の範囲ごとに描画内容を切り替える
    if (mode >= 400 && mode < 410) {
        // 待機 / 初期化完了表示
        canvas.drawCentreString("Motor - Standby", 160, 10 + y_off);

        // // 基本コントロールだけ表示（初期状態でも現在の設定を確認できるように）
        // _dir_toggle.draw(canvas);
        // _freqSlider.draw(canvas);

        // char buf[64];
        // std::sprintf(buf, "Freq: %d Hz", _current_frequency);
        // canvas.setTextSize(1);
        // canvas.setTextColor(WHITE);
        // canvas.drawString(buf, 40 + 6, (140 - 28) + y_off);

        // // ステータス表示（例: セルフチェック進行中フラグ等）
        // if (mode == 401) {
        //     canvas.drawCentreString("Self-check...", 160, 30 + y_off);
        // }

    } else if (mode >= 410 && mode < 420) {
        // デバイス選択 / 制御 UI
        canvas.drawCentreString("Motor - Control", 160, 10 + y_off);

        // ここでは選択中のモーター名や詳細を表示する余地がある
        // （実装側で mode==411 などに応じて表示を変える）
        if (mode == 411) canvas.drawString("Motor 1 selected", 10, 30 + y_off);
        else if (mode == 412) canvas.drawString("Motor 2 selected", 10, 30 + y_off);
        else canvas.drawString("Select motor to control", 10, 30 + y_off);

        // 制御パネル表示
        _dir_toggle.draw(canvas);//トグルスイッチ表示
        _freqSlider.draw(canvas);//スライダー表示

        char buf2[64];
        std::sprintf(buf2, "Freq: %d Hz", _current_frequency);
        canvas.setTextSize(1);
        canvas.setTextColor(WHITE);
        canvas.drawString(buf2, 40 + 6, (140 - 28) + y_off);

    } else {
        // その他モード（フォールバック）: 基本表示を行う
        _dir_toggle.draw(canvas);
        _freqSlider.draw(canvas);
        char buf3[64];
        std::sprintf(buf3, "Freq: %d Hz", _current_frequency);
        canvas.drawString(buf3, 40 + 6, (140 - 28) + y_off);
    }

    // 共通のボタン描画
    btnMgr.updateAll();
    btnMgr.drawAll(canvas);
}

/**
 * @brief タッチイベント（汎用）
 *
 * 画面のどこがタッチされたかに応じて `mode` を変更したり、
 * UI ウィジェットへイベントを渡す実装をここに追加します。
 */
void AppMotor::onTouch(int x, int y) {

}

void AppMotor::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

/**
 * @brief タッチ開始（押下）ハンドラ
 *
 * 例: 待機状態 (mode 400) のときにタッチでデバイス選択を始めるなら
 *       mode = 410 に設定する処理をここに書く
 */
void AppMotor::handlePress(int16_t x, int16_t y) {

}

void AppMotor::handleMove(int16_t x, int16_t y) {

}

void AppMotor::handleRelease(int16_t x, int16_t y) {

    // スライダーの変更通知は setOnChange コールバックで処理される
}

/**
 * @brief ホーム画面用アイコン描画
 */
void AppMotor::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    // 緑のアイコン例
    uint16_t bg = pressed ? DARKGREEN : GREEN;
    canvas.fillRoundRect(x, y, w, h, 10, bg);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
}


