#include "Button.h"
#include <M5CoreS3.h>
#include "system/touch/TouchManager.h"

/**
 * @brief CoreS3Buttons のコンストラクタ
 * 
 * @param label ボタンのラベル（表示される文字列）
 * @param x ボタンの左上X座標
 * @param y ボタンの左上Y座標
 * @param w ボタンの幅（ピクセル）
 * @param h ボタンの高さ（ピクセル）
 * @param color_bg ボタンの背景色（未押下時）
 * @param color_pressed ボタンの背景色（押下時）
 * @param color_text ボタンのテキスト色
 */
CoreS3Buttons::CoreS3Buttons(const char* label,int x, int y, int w, int h,
                uint16_t color_bg , uint16_t color_pressed , uint16_t color_text)
  : label_(label ? label : ""), x_(x), y_(y), w_(w), h_(h), pressed_(false),
    prev_touch_state_(-1), callback_(nullptr),
    color_bg_(color_bg), color_pressed_(color_pressed), color_text_(color_text),
    was_pressed_inside_(false), longpress_callback_(nullptr), long_press_ms_(500),
    long_press_triggered_(false), press_start_ms_(0) {
  has_icon_ = false;
  icon_color_ = color_text_;
  icon_type_ = IconType::None;
}

/**
 * @brief 初期化処理
 * 
 * 必要に応じて初期化コードをここに追加
 */
void CoreS3Buttons::begin() {
  // 初期化コード（現在は特に処理なし）
}

/**
 * @brief 毎フレーム実行 - タッチ入力の処理
 * 
 * 実行フロー：
 * 1. タッチマネージャから現在のタッチ座標と状態を取得
 * 2. タッチ位置がボタン領域内か判定
 * 3. ボタン領域内でのタッチ開始時：pressed_ フラグを立てる
 * 4. タッチ中（継続）：PRESSING イベントが設定されていればコールバック実行
 * 5. タッチ終了時：was_pressed_inside_ フラグに基づいてイベント判定
 *    - CLICK イベント：タッチしてから離した場合に実行
 *    - RELEASE イベント：タッチ終了時に実行
 * 6. ボタン外のタッチはキャンセル
 */
void CoreS3Buttons::update() {
  // タッチマネージャから現在のタッチ情報を取得
  int touch_x = touchManager.getX();
  int touch_y = touchManager.getY();
  bool touch_now = touchManager.isTouched();
  int touch_state = touchManager.getState();
  int touch_count = touchManager.getCount();

  // タッチ位置がボタン領域内か判定
  bool inside = touch_now &&
                (touch_x >= x_ && touch_x <= x_ + w_) &&
                (touch_y >= y_ && touch_y <= y_ + h_);

  if (touch_now) {
    // タッチが発生している状態
    if (inside) {
      // ボタン領域内でのタッチ
      if (!pressed_) {
        // 新規にボタンが押された
        pressed_ = true;
        was_pressed_inside_ = true;
        
        // PRESS イベント設定時：即座にコールバック実行
        if (callback_ && callback_trigger_ == EVENT_TYPE::PRESS) callback_();
      }
      
      // PRESSING イベント設定時：タッチ中、毎フレーム実行
      if (pressed_ && callback_ && callback_trigger_ == EVENT_TYPE::PRESSING) callback_();
    } else {
      // ボタン領域外でのタッチ -> プレス追跡をキャンセル
      pressed_ = false;
      was_pressed_inside_ = false;
    }
  } else {
    // タッチが解放された状態
    if (was_pressed_inside_) {
      // ボタン領域内でのプレスが解放された
      
      // CLICK イベント設定時：タップ（押して離す）で実行
      if (callback_trigger_ == EVENT_TYPE::CLICK) {
        if (callback_) callback_();
      }
      
      // Release コールバック（別設定）があれば実行
      if (release_callback_) release_callback_();
      
      // RELEASE イベント設定時：タッチ終了で実行
      if (callback_trigger_ == EVENT_TYPE::RELEASE) {
        if (callback_) callback_();
      }
    }
    
    // 状態をリセット
    pressed_ = false;
    was_pressed_inside_ = false;
  }

  // 前フレームのタッチ状態を保存（簡易的な状態追跡）
  prev_touch_state_ = touch_count > 0 ? touch_state : 0;
}

/**
 * @brief コールバック関数を設定
 * 
 * @param cb 実行するコールバック関数
 * @param trigger コールバックを実行するイベントタイプ（デフォルト: CLICK）
 * 
 * 対応するイベント：
 * - PRESS: ボタンが押された直後
 * - CLICK: タップ（押して離す）
 * - RELEASE: タッチが解放された時
 * - PRESSING: タッチ中、毎フレーム
 */
void CoreS3Buttons::setCallback(Callback cb, EVENT_TYPE trigger) {
  callback_ = std::move(cb);
  callback_trigger_ = trigger;
}

/**
 * @brief ボタンが現在押下状態か判定
 * 
 * @return true: 押下中、false: 押下していない
 */
bool CoreS3Buttons::isPressed() const {
  return pressed_;
}

/**
 * @brief ロングプレス時のコールバックを設定
 * 
 * @param cb ロングプレス検出時に実行するコールバック関数
 */
void CoreS3Buttons::setOnLongPress(Callback cb) {
  longpress_callback_ = std::move(cb);
}

/**
 * @brief タッチ解放時のコールバックを設定
 * 
 * @param cb タッチが解放された時に実行するコールバック関数
 */
void CoreS3Buttons::setOnRelease(Callback cb) {
  release_callback_ = std::move(cb);
}

/**
 * @brief ロングプレス判定の時間閾値を設定
 * 
 * @param ms ロングプレスと判定する時間（ミリ秒）
 */
void CoreS3Buttons::setLongPressMs(uint32_t ms) {
  long_press_ms_ = ms;
}

/**
 * @brief ボタン左側に表示するアイコンの色を設定
 * 
 * @param color アイコン色
 */
void CoreS3Buttons::setIconColor(uint16_t color) {
  has_icon_ = true;
  icon_color_ = color;
}

/**
 * @brief ボタンに特殊なアイコンタイプを設定
 * 
 * @param t アイコンタイプ（None: なし、Lock: ロックアイコン）
 */
void CoreS3Buttons::setIconType(IconType t) {
  icon_type_ = t;
  if (t == IconType::None) has_icon_ = false;
  else has_icon_ = true;
}