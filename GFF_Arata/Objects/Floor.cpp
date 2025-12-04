#include "Floor.h"
#include <cmath>

Floor::Floor() {
    x = 0;
    y = 600;     // 画面下部に配置
    width = 1280;
    height = 120;
    handle = -1;
}

void Floor::Initialize() {
    handle = LoadGraph("Resource/Background/Floor.png");
}

void Floor::Draw(float cameraOffsetX) {
    // カメラオフセットを適用して描画
    // マップ全体に地面を繰り返し描画（横スクロール対応）
    const float mapWidth = 4000.0f;
    const float screenW = 1280.0f;
    
    // 画面に表示される範囲の地面を描画
    float startX = cameraOffsetX;
    float endX = cameraOffsetX + screenW;
    
    // 地面のタイルを繰り返し描画
    float tileStartX = floor(startX / width) * width;
    for (float tileX = tileStartX; tileX < endX; tileX += width) {
        float drawX = tileX - cameraOffsetX;
        DrawExtendGraph((int)drawX, (int)y, (int)(drawX + width), (int)(y + height), handle, TRUE);
    }
}
