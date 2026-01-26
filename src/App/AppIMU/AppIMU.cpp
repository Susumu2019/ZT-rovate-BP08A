#include "AppIMU.h"
#include "config.h"
#include "../../system/system.h"
#include <math.h>

// IMU接続状態フラグ（外部定義）
extern bool imu6886_connected;

namespace {
struct Point3D { float x, y, z; };
struct Point2D { float x, y; };
struct RotationMatrix { float m[3][3]; };
constexpr float kPI = 3.14159265f;

// Roll(X軸), Pitch(Y軸), Yaw(Z軸)の標準的な回転行列
RotationMatrix getRotationMatrix(float rollRad, float pitchRad, float yawRad) {
    float cr = cosf(rollRad),  sr = sinf(rollRad);
    float cp = cosf(pitchRad), sp = sinf(pitchRad);
    float cy = cosf(yawRad),   sy = sinf(yawRad);

    RotationMatrix r;
    // 順序: Yaw -> Pitch -> Roll (Z -> Y -> X)
    // 各行は出力ベクトル方向
    r.m[0][0] = cy * cp;
    r.m[0][1] = cy * sp * sr - sy * cr;
    r.m[0][2] = cy * sp * cr + sy * sr;

    r.m[1][0] = sy * cp;
    r.m[1][1] = sy * sp * sr + cy * cr;
    r.m[1][2] = sy * sp * cr - cy * sr;

    r.m[2][0] = -sp;
    r.m[2][1] = cp * sr;
    r.m[2][2] = cp * cr;
    return r;
}

Point3D rotatePoint(const Point3D& p, const RotationMatrix& rot) {
    Point3D out;
    out.x = rot.m[0][0] * p.x + rot.m[0][1] * p.y + rot.m[0][2] * p.z;
    out.y = rot.m[1][0] * p.x + rot.m[1][1] * p.y + rot.m[1][2] * p.z;
    out.z = rot.m[2][0] * p.x + rot.m[2][1] * p.y + rot.m[2][2] * p.z;
    return out;
}

Point2D project(const Point3D& p, float centerX, float centerY, float distance) {
    float z = distance + p.z;
    float factor = (z > 0.1f) ? (100.0f / z) : 0.0f;
    return { centerX + p.x * factor, centerY - p.y * factor };
}

void drawBar(M5Canvas& canvas, int x, int y, int w, int h, float value, float minVal, float maxVal, uint16_t color, const char* label) {
    canvas.setTextColor(WHITE);
    canvas.setTextSize(1);
    canvas.drawString(label, x, y);

    float norm = (value - minVal) / (maxVal - minVal);
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    int barW = (int)(w * norm);
    canvas.drawRect(x, y + 12, w, h, WHITE);
    if (barW > 0) {
        canvas.fillRect(x, y + 12, barW, h, color);
    }
}

void drawCube(M5Canvas& canvas, float rollRad, float pitchRad, float yawRad,
              float centerX, float centerY, float scale, float distance) {
    // 立方体の8つの頂点 (-1～1の範囲)
    const Point3D verts[8] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}
    };

    // 各面の頂点インデックス（底面, 右側面, 左側面, 他）
    const uint8_t faces[6][4] = {
        {0,1,2,3}, // 底面（z=-1）
        {1,5,6,2}, // 右側面（x=+1）
        {0,3,7,4}, // 左側面（x=-1）
        {4,5,6,7}, // 上面（z=+1）
        {0,1,5,4}, // 前面（y=-1）
        {3,2,6,7}  // 背面（y=+1）
    };
    const uint16_t faceColors[6] = {
        0x7BEF, // 底面グレー（RGB565: 0x7BEF）
        0xFBAE, // 右側面淡い赤（RGB565: 0xFBAE）
        0x6D1F, // 左側面淡い青（RGB565: 0x6D1F）
        0xFFFF, // 上面白
        0xFFE0, // 前面黄
        0x07FF  // 背面シアン
    };

    RotationMatrix rot = getRotationMatrix(rollRad, pitchRad, yawRad);
    Point3D v3d[8];
    Point2D proj[8];
    for (int i = 0; i < 8; ++i) {
        v3d[i] = rotatePoint({verts[i].x * scale, verts[i].y * scale, verts[i].z * scale}, rot);
        proj[i] = project(v3d[i], centerX, centerY, distance);
    }

    // 面ごとに塗りつぶし（Painter's Algorithmで簡易的に）
    for (int f = 0; f < 3; ++f) { // 指定面のみ塗りつぶし
        int idx0 = faces[f][0], idx1 = faces[f][1], idx2 = faces[f][2], idx3 = faces[f][3];
        canvas.fillTriangle((int)proj[idx0].x, (int)proj[idx0].y, (int)proj[idx1].x, (int)proj[idx1].y, (int)proj[idx2].x, (int)proj[idx2].y, faceColors[f]);
        canvas.fillTriangle((int)proj[idx0].x, (int)proj[idx0].y, (int)proj[idx2].x, (int)proj[idx2].y, (int)proj[idx3].x, (int)proj[idx3].y, faceColors[f]);
    }

    // 立方体の12本の辺
    const uint8_t edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0}, // 底面
        {4,5},{5,6},{6,7},{7,4}, // 上面
        {0,4},{1,5},{2,6},{3,7}  // 側面
    };
    for (auto& e : edges) {
        Point2D a = proj[e[0]];
        Point2D b = proj[e[1]];
        canvas.drawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, WHITE);
    }
}

void drawAxes(M5Canvas& canvas, float rollRad, float pitchRad, float yawRad,
              float centerX, float centerY, float scale, float distance) {
    // 単位ベクトルをIMU姿勢で回転させて矢印表示
    const float axisLen = scale * 1.4f;
    const Point3D axes[3] = {
        {axisLen, 0, 0},   // +X (Roll軸)
        {0, axisLen, 0},   // +Y (Pitch軸)
        {0, 0, axisLen}    // +Z (Yaw軸)
    };
    const uint16_t colors[3] = { RED, GREEN, BLUE };
    const char* labels[3] = { "X", "Y", "Z" };

    RotationMatrix rot = getRotationMatrix(rollRad, pitchRad, yawRad);
    Point2D origin2d = project({0, 0, 0}, centerX, centerY, distance);

    for (int i = 0; i < 3; ++i) {
        Point3D tip = rotatePoint(axes[i], rot);
        Point2D tip2d = project(tip, centerX, centerY, distance);

        canvas.drawLine((int)origin2d.x, (int)origin2d.y, (int)tip2d.x, (int)tip2d.y, colors[i]);

        // 簡易矢印: 先端に小さな三角形を描く代わりにドット＋ラベル
        canvas.fillCircle((int)tip2d.x, (int)tip2d.y, 3, colors[i]);
        canvas.setTextColor(colors[i]);
        canvas.drawString(labels[i], (int)tip2d.x + 4, (int)tip2d.y - 10);
    }
}
}


AppIMU::AppIMU() : upAxis(UP_Z) {}

void AppIMU::setup() {
    mode = 200;
}

void AppIMU::loop() {
    // IMU更新はmain.cppで実施
}

void AppIMU::draw(M5Canvas &canvas) {
    canvas.fillScreen(BLACK);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(1);

    // IMU接続チェック
    if (!imu6886_connected) {
        canvas.setTextColor(RED);
        canvas.drawString("ERROR: IMU NOT CONNECTED", 10, 100);
        canvas.drawString("Check I2C connection", 10, 130);
        return;
    }

    // センサー更新は main.cpp loop() で行うため、ここでは呼ばない
    // imu6886_ahrs.update();  // 削除：ここで update しないこと
    
    float rollDeg = imu6886_ahrs.getRoll();// 度数法, 0～360 度
    float pitchDeg = imu6886_ahrs.getPitch();// 度数法, -180～180 度
    float yawDeg = imu6886_ahrs.getYaw();// 度数法, 0～360 度

    // 生センサー値取得（フィルタ適用前）
    float rawAccelX, rawAccelY, rawAccelZ;
    float rawGyroX, rawGyroY, rawGyroZ;
    imu6886_ahrs.sensor().readAccel(&rawAccelX, &rawAccelY, &rawAccelZ);
    imu6886_ahrs.sensor().readGyro(&rawGyroX, &rawGyroY, &rawGyroZ);

    // ラジアン変換
    const float rollRad = rollDeg * kPI / 180.0f;
    const float pitchRad = pitchDeg * kPI / 180.0f;
    const float yawRad = yawDeg * kPI / 180.0f;

    // === 上部: フィルタ済み値（左側）と生値（右側）の比較表示 ===
    canvas.drawString("=== FILTERED (L) vs RAW (R) ===", 10, 8);
    
    char buf[40];
    // Roll比較
    canvas.setTextColor(CYAN);
    sprintf(buf, "R:%.1f", rollDeg);
    canvas.drawString(buf, 10, 24);
    canvas.setTextColor(YELLOW);
    sprintf(buf, "AccX:%.2f", rawAccelX);
    canvas.drawString(buf, 160, 24);
    
    // Pitch比較
    canvas.setTextColor(CYAN);
    sprintf(buf, "P:%.1f", pitchDeg);
    canvas.drawString(buf, 10, 40);
    canvas.setTextColor(YELLOW);
    sprintf(buf, "AccY:%.2f", rawAccelY);
    canvas.drawString(buf, 160, 40);
    
    // Yaw比較
    canvas.setTextColor(CYAN);
    sprintf(buf, "Y:%.1f", yawDeg);
    canvas.drawString(buf, 10, 56);
    canvas.setTextColor(YELLOW);
    sprintf(buf, "AccZ:%.2f", rawAccelZ);
    canvas.drawString(buf, 160, 56);

    // === ジャイロ値表示 ===
    canvas.setTextColor(WHITE);
    canvas.drawString("---Gyro (dps)---", 10, 80);
    canvas.setTextColor(CYAN);
    sprintf(buf, "GyrX:%.1f", rawGyroX);
    canvas.drawString(buf, 10, 96);
    canvas.setTextColor(YELLOW);
    sprintf(buf, "GyrY:%.1f", rawGyroY);
    canvas.drawString(buf, 160, 96);
    
    // === フィルタ情報 ===
    float temperature = imu6886_ahrs.getTemperature();
    float gyroBiasX = imu6886_ahrs.getGyroBiasX();
    
    canvas.setTextColor(GREEN);
    sprintf(buf, "Temp:%.1fC BiasX:%.3f", temperature, gyroBiasX);
    canvas.drawString(buf, 10, 120);

    // === 下部: 3Dキューブ描画（画面中央に大きく表示） ===
    const float centerX = 200.0f;
    const float centerY = 165.0f;
    const float scale   = 24.5f; // 70% of 35.0
    const float distance = 80.0f;

    // 軸選択ボタン描画
    const int btnW = 60, btnH = 25, btnX = 100 - btnW - 10;
    const int btnY[3] = { 130, 160, 190 };
    const char* btnLabel[3] = { "X-UP", "Y-UP", "Z-UP" };
    for (int i = 0; i < 3; ++i) {
        uint16_t col = (upAxis == i) ? YELLOW : DARKGREY;
        canvas.fillRoundRect(btnX, btnY[i], btnW, btnH, 8, col);
        canvas.setTextColor(BLACK);
        canvas.drawCentreString(btnLabel[i], btnX + btnW/2, btnY[i] + 6);
    }

    // upAxisごとにcubeの初期姿勢だけを変え、IMUの回転軸の意味は絶対に変えない
    float roll0 = rollRad, pitch0 = pitchRad, yaw0 = yawRad;
    if (upAxis == UP_X) {
        // X軸を上に（Y軸中心に-90度回転）
        pitch0 -= kPI/2;
    } else if (upAxis == UP_Y) {
        // Y軸を上に（X軸中心に+90度回転）
        roll0 += kPI/2;
    } else {
        // Z軸を上に（デフォルト）
        // 何も回転しない
    }
    drawCube(canvas, roll0, pitch0, yaw0, centerX, centerY, scale, distance);
    drawAxes(canvas, roll0, pitch0, yaw0, centerX, centerY, scale, distance);
    // 画面下部に凡例
    canvas.setTextColor(CYAN);
    canvas.setTextSize(1);
    canvas.drawString("CYAN=Filtered", 10, 230);
    canvas.setTextColor(YELLOW);
    canvas.drawString("YEL=Raw", 180, 230);
}

int AppIMU::buttonHitTest(int x, int y) const {
    // draw()のボタン描画と同じ座標・サイズに合わせる
    const int btnW = 60, btnH = 25;
    const int btnX = 30; // 100 - btnW - 10
    const int btnY[3] = { 130, 160, 190 };
    for (int i = 0; i < 3; ++i) {
        if (x >= btnX && x < btnX + btnW && y >= btnY[i] && y < btnY[i] + btnH) return i;
    }
    return -1;
}

void AppIMU::onTouch(int x, int y) {
    int btn = buttonHitTest(x, y);
    if (btn >= 0) {
        upAxis = static_cast<UpAxis>(btn);
    }
}

void AppIMU::handleTouch(int16_t x, int16_t y) {
    onTouch((int)x, (int)y);
}

void AppIMU::drawIcon(M5Canvas& canvas, int x, int y, int w, int h, bool pressed) {
    uint16_t bg = pressed ? DARKGREEN : GREEN;
    canvas.fillRoundRect(x, y, w, h, 10, bg);
    canvas.setTextColor(WHITE);
    canvas.setTextSize(2);
    canvas.drawCentreString(appName(), x + w/2, y + h/2 - 8);
}

uint16_t AppIMU::iconBackgroundColor() const { return DARKGREEN; }
uint16_t AppIMU::iconPressedColor() const { return DARKGREY; }
uint16_t AppIMU::iconTextColor() const { return WHITE; }
