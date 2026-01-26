#include "HomeScreen.h"
#include <M5CoreS3.h>
#include <sstream>
#include "system/system.h"

/**
 * @brief HomeScreen のコンストラクタ
 */
HomeScreen::HomeScreen() {}

/**
 * @brief 初期化処理
 * 
 * mode 値を設定する
 */
void HomeScreen::setup() {
    // Serial.println("HomeScreen: setup called");
    mode = 150;
}

/**
 * @brief 毎フレーム実行
 * 
 * ボタンマネージャを更新してタップを検知
 */
void HomeScreen::loop() {
    btnMgr.updateAll();
}

/**
 * @brief 画面描画
 * 
 * @param canvas 描画対象の Canvas
 * 
 * 実行フロー：
 * 1. 画面幅に応じてアイコンの位置と大きさを計算
 * 2. 登録されたアプリをループして、各アプリのボタンを作成
 * 3. 各ボタンにタップ時のコールバックを設定
 * 4. 計算された位置にボタンを配置
 * 5. すべてのボタンを描画
 */
void HomeScreen::draw(M5Canvas& canvas) {
    btnMgr.clear();

    // 横方向のアイコン間隔を計算
    // cols 個のアイコンが画面幅に均等に並ぶように計算
    int screenW = canvas.width(); // 通常は 320
    int effectiveIconW = iconW; // アイコン幅（必要に応じて調整）
    int hGap = (screenW - cols * effectiveIconW) / (cols + 1); // アイコン間のギャップ幅
    
    // ギャップが小さすぎる場合はアイコン幅を減らして調整
    const int kMinGap = 4;
    if (hGap < kMinGap) {
        effectiveIconW = (screenW - kMinGap * (cols + 1)) / cols;
        if (effectiveIconW < 16) effectiveIconW = 16; // 最小幅
        hGap = (screenW - cols * effectiveIconW) / (cols + 1);
    }

    // アイコンの配置開始位置
    int startX = hGap;
    int startY = padding + topOffset; // トップオフセットでアイコンを下げる
    int x = startX;
    int y = startY;

    // 登録されたアプリの数だけボタンを作成して配置
    for (size_t i = 0; i < appList.size(); ++i) { // appList は setAppList で設定される
        App* app = appList[i];
        if (!app) continue; // NULL チェック
        
        // アプリ名、色情報を使ってボタンを作成
        // 変数名を `b` から `button` に変更して可読性を向上
        CoreS3Buttons button(app->appName(), x, y, effectiveIconW, iconH,
                     app->iconBackgroundColor(), app->iconPressedColor(), app->iconTextColor());
        
        // ボタンタップ時のコールバック設定
        // アプリのインデックスを渡して、AppManager に起動リクエストを通知
        int idx = (int)i;
        button.setCallback([this, idx]() {
            // Serial.printf("HomeScreen: button idx=%d clicked\r\n", idx);
            if (launchCallback) launchCallback(idx);
        });

        // ボタンマネージャに追加
        btnMgr.addButton(std::move(button));

        // 次のアイコン位置を計算
        x += effectiveIconW + hGap;
        
        // 行が満杯になったら次の行へ
        if ((i + 1) % cols == 0) {
            x = startX;
            y += iconH + padding + 16;
        }
    }
    
    // ボタン状態を最新化
    btnMgr.updateAll();
    
    // 画面を黒で塗りつぶして、すべてのボタンを描画
    canvas.fillScreen(BLACK);
    btnMgr.drawAll(canvas);
}

/**
 * @brief 表示するアプリのリストを設定
 * 
 * @param apps アプリポインタのベクトル
 */
void HomeScreen::setAppList(const std::vector<App*>& apps) {
    appList = apps;
}

/**
 * @brief アプリ起動時のコールバック関数を設定
 * 
 * @param cb アプリのインデックスを受け取るコールバック関数
 */
void HomeScreen::setLaunchCallback(std::function<void(int)> cb) {
    launchCallback = cb;
}
