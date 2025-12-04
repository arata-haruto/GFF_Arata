#include "InGameScene.h"
#include "../SceneFactory.h"
#include "../../Utility/InputManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/ItemManager.h"
#include "../../Utility/ReasoningManager.h"
#include "../../Utility/ReasoningUI.h"
#include "../../Utility/Vector2D.h"
#include "../../Objects/Player.h"
#include "../../Objects/Floor.h"
#include "../../Objects/Item.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <chrono>

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 10;
int mapData[MAP_HEIGHT][MAP_WIDTH];

Item* items[3];

Floor g_floor;

bool LoadMapData(const char* filename);

ItemManager itemManager;

InGameScene::InGameScene()
    : currentPhase(GamePhase::Opening),
    timeLimit(120.0f),
    remainingTime(120.0f),
    allEvidenceCollected(false),
    reasoningManager(nullptr),
    reasoningUI(nullptr),
    showResult(false),
    isCorrect(false),
    resultDisplayTime(0.0f),
    cameraX(0.0f)
{
}

void InGameScene::Initialize()
{
    LoadMapData("map.csv");

    player1 = new Player();
    player1->Initialize();
    g_floor.Initialize();

    itemManager.Add(new Item(400, 500, "証拠1", "証拠欄に追加された"));
    itemManager.Add(new Item(800, 500, "証拠2", "証拠欄に追加された"));
    itemManager.Add(new Item(1200, 500, "証拠3", "証拠欄に追加された"));
    itemManager.Add(new Item(1600, 480, "証拠4", "証拠欄に追加された"));
    itemManager.Add(new Item(2000, 520, "証拠5", "証拠欄に追加された"));
    itemManager.Add(new Item(2400, 490, "証拠6", "証拠欄に追加された"));
    itemManager.Add(new Item(2800, 510, "証拠7", "証拠欄に追加された"));
    itemManager.Add(new Item(3200, 500, "証拠8", "証拠欄に追加された"));
    itemManager.Init();

    reasoningManager = new ReasoningManager();
    reasoningManager->Initialize();
    reasoningManager->SetActive(false);

    // UI
    reasoningUI = new ReasoningUI();
    reasoningUI->Initialize();

    currentPhase = GamePhase::Opening;
    timeLimit = 120.0f;
    remainingTime = timeLimit;
    allEvidenceCollected = false;
    cameraX = 0.0f;

    // BGM
    mainbgm = LoadSoundMem("Resource/Sound/BGM.mp3");
    if (mainbgm != -1) {
        PlaySoundMem(mainbgm, DX_PLAYTYPE_LOOP);
    }

    back_ground_image = LoadGraph("Resource/Background/BG.jpg");
    if (back_ground_image == -1) {
        printfDx("読み込めない\n");
    }
}

eSceneType InGameScene::Update(float delta_second)
{
    InputManager* input = InputManager::GetInstance();

    if (currentPhase == GamePhase::Opening) {

        if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed)
        {
            currentPhase = GamePhase::EvidenceCollection;
        }
    }
    else if (currentPhase == GamePhase::EvidenceCollection) {

        if (player1) player1->Update();

        if (input->GetKeyState(KEY_INPUT_A) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_START) == eInputState::Pressed)
        {
            return eSceneType::eTitle;
        }

        if (input->GetKeyState(KEY_INPUT_E) == eInputState::Pressed) {
            itemManager.ToggleList();
        }

        itemManager.Update(player1->GetX(), player1->GetY(), delta_second);

        const float screenW = 1280.0f;
        const float mapWidth = 4000.0f;
        float playerX = player1->GetX();

        float targetCameraX = playerX - screenW / 2.0f;
        if (targetCameraX < 0.0f) targetCameraX = 0.0f;
        if (targetCameraX > mapWidth - screenW) targetCameraX = mapWidth - screenW;

        cameraX = targetCameraX;

        remainingTime -= delta_second;
        if (remainingTime < 0.0f) remainingTime = 0.0f;

        if (itemManager.GetCollectedCount() >= itemManager.GetTotalCount()) {
            allEvidenceCollected = true;
        }

        if (remainingTime <= 0.0f || allEvidenceCollected) {
            TransitionToReasoning();
        }
    }
    else if (currentPhase == GamePhase::Reasoning) {

        if (showResult) {
            resultDisplayTime += delta_second;
            if (resultDisplayTime >= 3.0f) {
                return eSceneType::eTitle;
            }
        }
        else if (reasoningManager) {

            reasoningManager->Update(delta_second);

            if (reasoningManager->IsConfirmed()) {
                const ReasoningOption& selected = reasoningManager->GetSelectedOption();
                isCorrect = CheckAnswer(selected);
                showResult = true;
                resultDisplayTime = 0.0f;
                reasoningManager->ResetConfirmed();
            }
        }
    }

    return GetNowSceneType();
}

void InGameScene::Draw() const
{
    if (currentPhase == GamePhase::Opening) {
        DrawOpening();
    }
    else if (currentPhase == GamePhase::EvidenceCollection) {

        if (back_ground_image != -1) {
            DrawGraph(0, 0, back_ground_image, TRUE);
        }

        g_floor.Draw(cameraX);

        itemManager.Draw(cameraX);

        if (player1) player1->Draw(cameraX);

        DrawTimer();
        DrawPhaseInfo();
    }
    else if (currentPhase == GamePhase::Reasoning) {

        if (showResult) {
            DrawResult();
        }
        else {

            if (back_ground_image != -1) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                DrawGraph(0, 0, back_ground_image, TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }

            if (reasoningUI && reasoningManager) {
                reasoningUI->Draw(reasoningManager);
            }
        }
    }
}

void InGameScene::Finalize()
{
    // BGM停止
    if (mainbgm >= 0) {
        StopSoundMem(mainbgm);
        DeleteSoundMem(mainbgm);
        mainbgm = -1;
    }

    if (back_ground_image >= 0) {
        DeleteGraph(back_ground_image);
        back_ground_image = -1;
    }

    delete player1;
    player1 = nullptr;

    delete reasoningManager;
    reasoningManager = nullptr;

    delete reasoningUI;
    reasoningUI = nullptr;
}

bool LoadMapData(const char* filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        printfDx("Map読み込み失敗: %s\n", filename);
        return false;
    }

    std::string line;
    int y = 0;

    while (std::getline(file, line) && y < MAP_HEIGHT) {

        std::stringstream ss(line);
        std::string cell;
        int x = 0;

        while (std::getline(ss, cell, ',') && x < MAP_WIDTH) {
            mapData[y][x] = std::stoi(cell);
            x++;
        }
        y++;
    }

    file.close();
    return true;
}

eSceneType InGameScene::GetNowSceneType() const
{
    return eSceneType::eInGame;
}

void InGameScene::TransitionToReasoning()
{
    currentPhase = GamePhase::Reasoning;

    if (reasoningManager) {

        std::vector<std::string> collectedEvidence = itemManager.GetCollectedItems();

        reasoningManager->FilterOptions(collectedEvidence);
        reasoningManager->SetActive(true);

        if (reasoningUI) {
            reasoningUI->SetEvidenceList(collectedEvidence);
        }
    }
}

void InGameScene::DrawTimer() const
{
    int x = 20;
    int y = 20;

    int boxWidth = 200;
    int boxHeight = 60;

    DrawBox(x - 10, y - 10, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);

    DrawBox(x - 10, y - 10, x + boxWidth, y + boxHeight,
        GetColor(100, 100, 100), FALSE);

    int minutes = (int)(remainingTime / 60.0f);
    int seconds = (int)(remainingTime) % 60;

    unsigned int color =
        remainingTime < 60.0f ? GetColor(255, 0, 0) : GetColor(255, 255, 255);

    DrawFormatString(x, y, color, "Time: %02d:%02d", minutes, seconds);
}

void InGameScene::DrawPhaseInfo() const
{
    int x = 20;
    int y = 90;
    
    DrawFormatString(x, y, GetColor(255, 255, 255), "証拠: %d/%d", 
        itemManager.GetCollectedCount(), itemManager.GetTotalCount());
}

void InGameScene::DrawOpening() const
{
    int x = 100;
    int y = 100;
    int boxWidth = 1000;
    int boxHeight = 500;

    // 背景ボックス
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(255, 255, 255), FALSE);

    DrawFormatString(x, y - 30, GetColor(255, 255, 255), "オープニング");

    int currentY = y + 20;

    // === 事件サマリーページ ===
    std::vector<std::string> lines = {
        "【場所】",
        "・午前中某所 駅前の雑貨店",
        "",
        "【時間】",
        "・PM 10:30頃",
        "",
        "【事件サマリー】",
        "容疑者は店に入って倒れているところを発見されました。",
        "現場には大量の手紙があり、一部の品物は踏み落とされていました。",
        "",
        "【目撃情報】",
        "・周辺付近で悲鳴を聞いたという情報",
        "・店に付け巡る走り去る影を見たとの情報",
        "",
        "【あなたの目的】",
        "- 集めた証拠から真犯人を推理する",
        "- 不審な情報や矛盾点を調べる",
        "- 論理的に犯人を詰め込む",
        "",
        "Zキー または Aボタン で開始"
    };

    for (const auto& line : lines) {
        DrawFormatString(x, currentY, GetColor(255, 255, 255), line.c_str());
        currentY += 30;
    }
}

void InGameScene::DrawResult() const
{
    int x = 400;
    int y = 300;
    int boxWidth = 500;
    int boxHeight = 200;
    
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(255, 255, 255), FALSE);
    
    unsigned int titleColor = isCorrect ? GetColor(0, 255, 0) : GetColor(255, 0, 0);
    std::string title = isCorrect ? "正解!" : "不正解";
    
    DrawFormatString(x + 150, y, titleColor, "%s", title.c_str());
    
    if (isCorrect) {
        DrawFormatString(x + 50, y + 60, GetColor(255, 255, 255), "おめでとうございます!");
    }
    else {
        DrawFormatString(x + 50, y + 60, GetColor(255, 255, 255), "推理が間違っていました");
    }
    
    DrawFormatString(x + 100, y + 120, GetColor(200, 200, 200), "タイトルへ戻ります");
}

bool InGameScene::CheckAnswer(const ReasoningOption& selected)
{
    // 正解の設定：ReasoningManager::Initialize()で登録した「山田」「過去の因縁」
    std::string correctSuspect = "山田";
    std::string correctMotive = "過去の因縁";
    
    // 安全な文字列比較（空文字列チェック付き）
    if (selected.suspect.empty() || selected.motive.empty()) {
        return false;
    }
    
    return (selected.suspect == correctSuspect && selected.motive == correctMotive);
}
