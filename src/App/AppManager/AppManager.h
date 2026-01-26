#pragma once
#include <vector>
#include "App/App.h"
#include "UI/TopBar/TopBar.h"

class AppManager {
public:
    AppManager();
    ~AppManager();

    void registerApp(App* app);
    void switchToApp(int appId);
    void showHomeScreen();
    void loop();
    void draw(M5Canvas &canvas);
    void onTouch(int x, int y);
    // continuous touch events
    void onPress(int x, int y);
    void onMove(int x, int y);
    void onRelease(int x, int y);
    int getCurrentAppId() const;

    // アプリ初期化・ホーム画面セットアップ
    void initializeApps();
    
    // TopBarへのアクセス（通信状態更新用）
    TopBar& getTopBar() { return topBar; }

private:
    std::vector<App*> apps;
    App* currentApp;
    int currentAppId;
    TopBar topBar;
};
