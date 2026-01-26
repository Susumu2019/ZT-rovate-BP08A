#include "AppManager.h"
#include "../HomeScreen/HomeScreen.h"
#include "../AppWifi/AppWifi.h"
#include "../AppAction/AppAction.h"
#include "../AppIMU/AppIMU.h"
#include "../AppManual/AppManual.h"
#include "../AppMotor/AppMotor.h"
#include "../AppI2CScan/AppI2CScan.h"
#include "../AppSetup/AppSetup.h"
#include <M5CoreS3.h>
#include "system/touch/TouchManager.h"

#include <cstring>

AppManager::AppManager()
    : currentApp(nullptr), currentAppId(-1) {}

AppManager::~AppManager() {}

void AppManager::registerApp(App* app) {
    apps.push_back(app);
}

/**
 * @brief 指定したアプリに切り替える
 * @param appId 切り替え先のアプリID（登録順に0から始まる）
 */
void AppManager::switchToApp(int appId) {
    Serial.printf("AppManager: switchToApp requested id=%d\r\n", appId);
    if (appId >= 0 && appId < apps.size()) {
        // アプリを切り替え
        currentApp = apps[appId];
        currentAppId = appId;
        
        // TopBarのタイトルを更新
        if (currentApp) topBar.setTitle(String(currentApp->appName()));
        Serial.printf("AppManager: switched to id=%d (type=%s)\r\n", appId, currentApp->typeName());


        // AppWifiの場合はネットワーク情報を出力
        if (currentApp && strcmp(currentApp->typeName(), "AppWifi") == 0) {
            AppWifi* wifiApp = static_cast<AppWifi*>(currentApp);
            if (wifiApp) wifiApp->printNetworkInfo();
        }

        // 新しいアプリの初期化処理を実行
        currentApp->setup();
    } else {
        Serial.printf("AppManager: switchToApp id out of range=%d\r\n", appId);
    }
}

/**
 * @brief ホーム画面（アプリ一覧）を表示
 */
void AppManager::showHomeScreen() {
    // 登録済みアプリからHomeScreenを探して切り替え
    for (size_t i = 0; i < apps.size(); ++i) {
        if (apps[i] && strcmp(apps[i]->typeName(), "HomeScreen") == 0) {
            currentApp = apps[i];
            currentAppId = i;
            currentApp->setup();
            if (currentApp) topBar.setTitle(String(currentApp->appName()));
            break;
        }
    }
}

/**
 * @brief 全アプリの初期化処理
 * 
 * 実行フロー：
 * 1. 各アプリのインスタンスを作成（静的変数として保持）
 * 2. AppManagerに登録
 * 3. HomeScreenにアプリ一覧とコールバックを設定
 * 4. TopBarを初期化
 */
void AppManager::initializeApps() {
    // 各アプリのインスタンスを作成（静的変数なのでメモリが固定）
    static HomeScreen homeScreen;
    static AppWifi appwifi;
    static AppAction appaction;
    static AppIMU appimu;
    static AppManual appmanual;
    static AppMotor appmotor;
    static AppI2CScan appI2CScan;
    static AppSetup appSetup;

    // AppManagerにアプリを登録（登録順がIDになる）
    registerApp(&homeScreen);  // ID: 0
    registerApp(&appwifi);     // ID: 1
    registerApp(&appaction);   // ID: 2
    registerApp(&appimu);      // ID: 3
    registerApp(&appmanual);   // ID: 4
    registerApp(&appI2CScan);  // ID: 5
    registerApp(&appSetup);    // ID: 6
    // registerApp(&appmotor); // AppMotor は登録しない（必要なら有効化）


    // HomeScreenに表示するアプリ一覧を設定（HomeScreen自身は除く）
    std::vector<App*> displayApps;
    for (App* a : apps) {
        if (a && strcmp(a->typeName(), "HomeScreen") != 0) {
            displayApps.push_back(a);
        }
    }
    homeScreen.setAppList(displayApps);
    
    // HomeScreenからのアプリ起動コールバックを設定
    // displayAppsのインデックスから登録済みappsのインデックスにマップ
    homeScreen.setLaunchCallback([this, displayApps](int idx) {
        Serial.printf("AppManager: launch callback received idx=%d\r\n", idx);
        if (idx < 0 || idx >= (int)displayApps.size()) {
            Serial.printf("AppManager: idx out of range\r\n");
            return;
        }
        App* target = displayApps[idx];
        for (size_t i = 0; i < apps.size(); ++i) {
            if (apps[i] == target) {
                Serial.printf("AppManager: mapped display idx %d -> registered idx %d\r\n", idx, (int)i);
                this->switchToApp((int)i);
                return;
            }
        }
    });
    
    // 起動時は AppLock を最初に表示する（必要に応じて HomeScreen に切り替え可能）
    // 登録済みアプリの中から AppLock を探して切り替える
    bool switched = false;
    for (size_t i = 0; i < apps.size(); ++i) {
        if (apps[i] && strcmp(apps[i]->typeName(), "AppWifi") == 0) {
            this->switchToApp((int)i);
            switched = true;
            break;
        }
    }
    // AppWifi が見つからない場合は既定で HomeScreen を表示
    if (!switched) {
        showHomeScreen();
    }
    
    // TopBarの初期化
    topBar.begin();
}

/**
 * @brief メインループ処理（毎フレーム実行）
 * 
 * 実行フロー：
 * 1. タッチイベントの種類を判定（Press/Move/Release）
 * 2. イベントに応じてハンドラを呼び出し
 * 3. 現在のアプリのloop()を実行
 * 4. TopBarを更新（時刻など）
 */
void AppManager::loop() {
    // タッチマネージャから座標とイベント状態を取得
    int touch_x = touchManager.getX();
    int touch_y = touchManager.getY();
    bool was_pressed = touchManager.wasPressed();
    bool is_pressed = touchManager.isTouched();
    bool was_released = touchManager.wasReleased();
    
    // タッチイベントに応じてハンドラを呼び出し（Press → Move → Release の順序）
    if (was_pressed) {
        onPress(touch_x, touch_y);
    } else if (is_pressed) {
        onMove(touch_x, touch_y);
    } else if (was_released) {
        onRelease(touch_x, touch_y);
    }
    
    // 現在のアプリのメインロジックを実行
    if (currentApp) currentApp->loop();
    
    // TopBarを更新（時刻表示など）
    topBar.update();
}

/**
 * @brief 画面描画処理（毎フレーム実行）
 * 
 * アプリの内容を先に描画してから、TopBarを上に重ねることで
 * TopBarが上に表示されるようにしている
 */
void AppManager::draw(M5Canvas &canvas) {
    // 1. アプリの内容を描画
    // 注: 画面の見た目を編集したい場合は、各アプリの draw() メソッドを編集してください
    //     例えば:
    //     - HomeScreen の内容: src/App/HomeScreen/HomeScreen.cpp の draw() メソッド
    //     - AppLock の内容: src/App/AppLock/AppLock.cpp の draw() メソッド
    //     - AppInfo の内容: src/App/AppInfo/AppInfo.cpp の draw() メソッド
    //     - AppMotor の内容: src/App/AppMotor/AppMotor.cpp の draw() メソッド
    if (currentApp) currentApp->draw(canvas);
    
    // 2. TopBarをアプリの上に描画
    // TopBar（上部のステータスバー）の見た目を編集したい場合は、
    // src/UI/TopBar/TopBar.cpp の draw() メソッドを編集してください
    topBar.draw(canvas);
}

/**
 * @brief タッチイベントハンドラ（旧API・現在は未使用）
 * @deprecated 簡素化に伴い、onPress/onMove/onRelease を使用
 */
void AppManager::onTouch(int x, int y) {
    if (currentApp) currentApp->handleTouch((int16_t)x, (int16_t)y);
}

/**
 * @brief タッチ開始時のハンドラ
 * @param x タッチのX座標
 * @param y タッチのY座標
 */
void AppManager::onPress(int x, int y) {
    if (currentApp) currentApp->handlePress((int16_t)x, (int16_t)y);
}

/**
 * @brief タッチ移動時のハンドラ（タッチしながら移動）
 * @param x タッチのX座標
 * @param y タッチのY座標
 */
void AppManager::onMove(int x, int y) {
    if (currentApp) currentApp->handleMove((int16_t)x, (int16_t)y);
}

/**
 * @brief タッチ終了時のハンドラ（指を離した時）
 * @param x タッチのX座標
 * @param y タッチのY座標
 */
void AppManager::onRelease(int x, int y) {
    if (currentApp) currentApp->handleRelease((int16_t)x, (int16_t)y);
}

/**
 * @brief 現在のアプリIDを取得
 * @return 登録順に基づいたアプリのID（0以上）
 */
int AppManager::getCurrentAppId() const {
    return currentAppId;
}
