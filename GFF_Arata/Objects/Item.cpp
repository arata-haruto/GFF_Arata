#include "Item.h"
#include "../Utility/InputManager.h"
#include <cmath>


int Item::handle = -1;
static int itemGetSE = -1;


Item::Item(float posX, float posY, const std::string& n, const std::string& d)
    : x(posX), y(posY), isCollected(false),
    showMessage(false), messageTimer(0.0f),
    name(n), description(d)
{
}

void Item::Init() {
    if (handle == -1) {
        handle = LoadGraph("Resource/UI/icon/Item.png");
        if (handle == -1) {
            printfDx("Resource/Item.png\n");
        }
    }

    if (itemGetSE == -1) {
        itemGetSE = LoadSoundMem("Resource/Sound/GetItem.mp3");
        if (itemGetSE == -1) {
            printfDx("GetItem.mp3\n");
        }
    }
}

void Item::Update(float playerX, float playerY, float deltaTime) {
    if (isCollected) {
        if (showMessage) {
            messageTimer -= deltaTime;
            if (messageTimer <= 0.0f) {
                showMessage = false;
            }
        }
        return;
    }

    float dx = playerX - x;
    float dy = playerY - y;
    float distance = sqrtf(dx * dx + dy * dy);
    const float pickupRadius = 150.0f;

    if (distance < pickupRadius) {
        InputManager* input = InputManager::GetInstance();

        if (input->GetKeyState(KEY_INPUT_SPACE) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed) {

            isCollected = true;
            if (itemGetSE != -1) {
                PlaySoundMem(itemGetSE, DX_PLAYTYPE_BACK);
            }
            showMessage = true;
            messageTimer = 2.0f;
        }
    }
}

void Item::Draw(float cameraOffsetX) const {
    if (!isCollected) {
        float drawX = x - cameraOffsetX;
        DrawGraph((int)drawX, (int)y, handle, TRUE);
    }

    if (showMessage) {
        float drawX = x - cameraOffsetX;
        std::string message = name + " “üŽè";
        DrawFormatString((int)drawX, (int)y - 30, GetColor(255, 255, 0), "%s", message.c_str());
    }
}
