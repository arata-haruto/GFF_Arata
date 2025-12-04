#pragma once
#include "DxLib.h"

class Player {
private:
    float x, y;
    float speed;
    int handle;
    int flip_flag;

    int footSE = -1; //����

public:
    Player();
    void Initialize();
    void Update();
    void Draw(float cameraOffsetX = 0.0f);

    float GetX() const { return x; }
    float GetY() const { return y; }
};