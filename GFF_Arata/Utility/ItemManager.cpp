#include "ItemManager.h"
#include "DxLib.h"
#include "InputManager.h"
#include <algorithm>

struct NamedItem {
    Item* item;
    std::string name;
};

ItemManager::~ItemManager() {
    for (auto& item : items) {
        delete item;
    }
}

void ItemManager::Add(Item* item) {
    item->Init();
    items.push_back(item);
}

void ItemManager::Init() {
    mode = ItemMode::List;
    selectedIndex = 0;
    isListOpen = false;
}

void ItemManager::Update(float playerX, float playerY, float deltaTime) {
    for (size_t i = 0; i < items.size(); ++i) {
        items[i]->Update(playerX, playerY, deltaTime);
    }

    if (isListOpen) {
        InputManager* input = InputManager::GetInstance();
        
        // アイテムが空の場合は何もしない
        if (items.empty()) {
            return;
        }
        
        // selectedIndexの範囲チェック
        if (selectedIndex < 0 || selectedIndex >= (int)items.size()) {
            selectedIndex = 0;
        }

        if (mode == ItemMode::List) {
            // 上下キーで選ぶ
            if (input->GetKeyState(KEY_INPUT_UP) == eInputState::Pressed ||
                input->GetKeyState(KEY_INPUT_UP) == eInputState::Held ||
                input->GetButtonState(XINPUT_BUTTON_DPAD_UP) == eInputState::Pressed ||
                input->GetButtonState(XINPUT_BUTTON_DPAD_UP) == eInputState::Held) {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = (int)items.size() - 1;
            }
            if (input->GetKeyState(KEY_INPUT_DOWN) == eInputState::Pressed ||
                input->GetKeyState(KEY_INPUT_DOWN) == eInputState::Held ||
                input->GetButtonState(XINPUT_BUTTON_DPAD_DOWN) == eInputState::Pressed ||
                input->GetButtonState(XINPUT_BUTTON_DPAD_DOWN) == eInputState::Held) {
                selectedIndex++;
                if (selectedIndex >= (int)items.size()) selectedIndex = 0;
            }

            // Zキーで詳細モードへ
            if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
                input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed) {
                mode = ItemMode::Detail;
            }
        }
        else if (mode == ItemMode::Detail) {
            // Xキーでリストに戻る
            if (input->GetKeyState(KEY_INPUT_X) == eInputState::Pressed ||
                input->GetButtonState(XINPUT_BUTTON_B) == eInputState::Pressed) {
                mode = ItemMode::List;
            }
        }
    }
}

void ItemManager::Draw(float cameraOffsetX) const {
    for (auto& item : items) {
        item->Draw(cameraOffsetX);
    }

    if (isListOpen) {
        // アイテムが空の場合の処理
        if (items.empty()) {
            int x = 50;
            int y = 50;
            DrawBox(x - 20, y - 40, x + 300, y + 60,
                GetColor(0, 0, 0), TRUE);
            DrawBox(x - 20, y - 40, x + 300, y + 60,
                GetColor(255, 255, 255), FALSE);
            DrawFormatString(x, y, GetColor(255, 255, 255), "アイテムがありません");
            return;
        }
        
        // selectedIndexの範囲チェック
        int safeIndex = selectedIndex;
        if (safeIndex < 0 || safeIndex >= (int)items.size()) {
            safeIndex = 0;
        }
        
        int x = 50;
        int y = 50;
        int boxWidth = 400;
        int boxHeight = 0;

        if (mode == ItemMode::List) {
            boxHeight = 20 + (int)items.size() * 20;
        }
        else if (mode == ItemMode::Detail) {
            const Item* item = items[safeIndex];
            int descLength = (int)item->GetDescription().length();
            int descLines = (descLength / 50) + 1;
            boxHeight = 150 + descLines * 20;
        }

        DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
            GetColor(0, 0, 0), TRUE);
        DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
            GetColor(255, 255, 255), FALSE);

        if (mode == ItemMode::List) {
            DrawFormatString(x, y - 30, GetColor(255, 255, 255), "ItemList");
            for (int i = 0; i < (int)items.size(); i++) {
                int drawY = y + i * 20;
                
                if (i == safeIndex) {
                    DrawBox(x - 10, drawY - 2, x + boxWidth - 10, drawY + 18,
                        GetColor(50, 50, 150), TRUE);
                    DrawBox(x - 10, drawY - 2, x + boxWidth - 10, drawY + 18,
                        GetColor(100, 150, 255), FALSE);
                }
                
                if (i == safeIndex) {
                    DrawFormatString(x - 5, drawY, GetColor(255, 255, 0), ">");
                }
                
                DrawFormatString(x + 15, drawY, 
                    i == safeIndex ? GetColor(255, 255, 0) : GetColor(255, 255, 255),
                    "%s", items[i]->GetName().c_str());
            }
        }
        else if (mode == ItemMode::Detail) {
            const Item* item = items[safeIndex];
            DrawFormatString(x, y, GetColor(255, 255, 255), "Name: %s", item->GetName().c_str());
            
            std::string desc = item->GetDescription();
            int lineWidth = (boxWidth - 40) / 8;
            int currentY = y + 40;
            size_t pos = 0;
            while (pos < desc.length()) {
                size_t endPos = pos + lineWidth;
                if (endPos >= desc.length()) {
                    endPos = desc.length();
                }
                else {
                    std::string searchRange = desc.substr(pos, lineWidth);
                    size_t lastSpace = searchRange.find_last_of(" ,.");
                    if (lastSpace != std::string::npos && lastSpace > 0) {
                        endPos = pos + lastSpace + 1;
                    }
                }
                std::string line = desc.substr(pos, endPos - pos);
                DrawFormatString(x, currentY, GetColor(255, 255, 255), "%s", line.c_str());
                currentY += 20;
                pos = endPos;

                while (pos < desc.length() && desc[pos] == ' ') {
                    pos++;
                }
            }
            DrawFormatString(x, currentY + 20, GetColor(200, 200, 200), "Press X to back");
        }
    }
}

void ItemManager::ToggleList() {
    isListOpen = !isListOpen;
    if (isListOpen) {
        mode = ItemMode::List;
    }
}

std::vector<std::string> ItemManager::GetCollectedItems() const {
    std::vector<std::string> collected;
    for (const auto& item : items) {
        if (item->GetIsCollected()) {
            collected.push_back(item->GetName());
        }
    }
    return collected;
}

int ItemManager::GetCollectedCount() const {
    int count = 0;
    for (const auto& item : items) {
        if (item->GetIsCollected()) {
            count++;
        }
    }
    return count;
}
