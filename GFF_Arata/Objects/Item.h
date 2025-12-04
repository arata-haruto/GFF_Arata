#pragma once
#include "DxLib.h"
#include <string>

class Item {
private:
    static int handle;  // �摜�n���h��
    float x, y;         // ���W
    bool isCollected;   // �擾�ς݃t���O

    bool showMessage;     // ���b�Z�[�W��\������t���O
    float messageTimer;   // ���b�Z�[�W��\�����鎞�ԁi�b�j

    std::string name;       // �A�C�e����
    std::string description; // �A�C�e��������

public:
    Item(float posX, float posY, const std::string& n, const std::string& d);

    void Init();
    void Update(float playerX, float playerY, float deltaTime);
    void Draw(float cameraOffsetX = 0.0f) const;

    const std::string& GetName() const { return name; }
    const std::string& GetDescription() const { return description; }
    bool GetIsCollected() const { return isCollected; }
    void SetCollected(bool flag) { isCollected = flag; }
    float GetX() const { return x; }
    float GetY() const { return y; }
};