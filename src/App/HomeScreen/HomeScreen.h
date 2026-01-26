#pragma once
#include "App/App.h"
#include <vector>
#include <functional>
#include "UI/Button/Button.h"

/**
 * @class HomeScreen
 * @brief ホーム画面 - アプリ一覧を表示するアプリ
 * 
 * M5CoreS3の起動時に最初に表示される画面
 * 登録されたアプリをアイコンボタンとして表示し、タップでアプリを起動
 */
class HomeScreen : public App {
public:
    HomeScreen();
    
    /** @brief 初期化処理（モード値を設定） */
    void setup() override;
    
    /** @brief 毎フレーム実行（ボタンマネージャの更新） */
    void loop() override;
    
    /** @brief 画面描画（アイコンボタンを描画） */
    void draw(M5Canvas& canvas) override;

    /** @brief 表示するアプリのリストを設定 */
    void setAppList(const std::vector<App*>& apps);
    
    /** @brief アプリ起動時のコールバック関数を設定 */
    void setLaunchCallback(std::function<void(int)> cb);
    
    const char* typeName() const override { return "HomeScreen"; }
    const char* appName() const override { return "Home"; }
    
private:
    /** @brief 表示するアプリのリスト */
    std::vector<App*> appList;
    
    /** @brief アプリ起動時に呼び出されるコールバック（アプリインデックスを受け取る） */
    std::function<void(int)> launchCallback;
    
    /** @brief ボタン（アイコン）管理 */
    ButtonManager btnMgr;

    // レイアウト設定
    int cols = 3;  // 横に並べるアイコン数
    int iconW = 80;  // アイコン幅（ピクセル）
    int iconH = 80;  // アイコン高さ（ピクセル）
    int padding = 12;  // アイコン間のパディング（ピクセル）
    int topOffset = 16;  // 上からのオフセット（ピクセル）
};
