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

    itemManager.Add(new Item(400, 500, "�؋�1", "�؋�����������"));
    itemManager.Add(new Item(800, 500, "�؋�2", "�؋�����������"));
    itemManager.Add(new Item(1200, 500, "�؋�3", "�؋�����������"));
    itemManager.Add(new Item(1600, 480, "�؋�4", "�؋�����������"));
    itemManager.Add(new Item(2000, 520, "�؋�5", "�؋�����������"));
    itemManager.Add(new Item(2400, 490, "�؋�6", "�؋�����������"));
    itemManager.Add(new Item(2800, 510, "�؋�7", "�؋�����������"));
    itemManager.Add(new Item(3200, 500, "�؋�8", "�؋�����������"));
    itemManager.Init();

    reasoningManager = new ReasoningManager();
    reasoningManager->Initialize();
    reasoningManager->SetActive(false);

    // UI������
    reasoningUI = new ReasoningUI();
    reasoningUI->Initialize();

    currentPhase = GamePhase::Opening;
    timeLimit = 120.0f;
    remainingTime = timeLimit;
    allEvidenceCollected = false;
    cameraX = 0.0f;

    // BGM�ǂݍ���
    mainbgm = LoadSoundMem("Resource/Sound/BGM.mp3");
    if (mainbgm != -1) {
        PlaySoundMem(mainbgm, DX_PLAYTYPE_LOOP);
    }

    back_ground_image = LoadGraph("Resource/Background/BG.jpg");
    if (back_ground_image == -1) {
        printfDx("�ǂݍ��߂Ȃ�\n");
    }
}

eSceneType InGameScene::Update(float delta_second)
{
    InputManager* input = InputManager::GetInstance();

    if (currentPhase == GamePhase::Opening) {
        // �I�[�v�j���O��ʂ�Z�L�[�܂���A�{�^���ŃQ�[���J�n
        if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed)
        {
            currentPhase = GamePhase::EvidenceCollection;
        }
    }
    else if (currentPhase == GamePhase::EvidenceCollection) {
        // �v���C���[�̍X�V
        if (player1) player1->Update();

        // �^�C�g���ɖ߂�iA�L�[�A�܂���START�{�^���j
        if (input->GetKeyState(KEY_INPUT_A) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_START) == eInputState::Pressed)
        {
            return eSceneType::eTitle;
        }

        // �A�C�e�����X�g�\���ؑցiE�L�[�j
        if (input->GetKeyState(KEY_INPUT_E) == eInputState::Pressed) {
            itemManager.ToggleList();
        }

        // �A�C�e���}�l�[�W���[�X�V
        itemManager.Update(player1->GetX(), player1->GetY(), delta_second);

        // �J�����Ǐ]����
        const float screenW = 1280.0f;
        const float mapWidth = 4000.0f;
        float playerX = player1->GetX();

        float targetCameraX = playerX - screenW / 2.0f;
        if (targetCameraX < 0.0f) targetCameraX = 0.0f;
        if (targetCameraX > mapWidth - screenW) targetCameraX = mapWidth - screenW;

        cameraX = targetCameraX;

        // �c�莞�Ԃ̍X�V
        remainingTime -= delta_second;
        if (remainingTime < 0.0f) remainingTime = 0.0f;

        // ���ׂĂ̏؋����W�܂������`�F�b�N
        if (itemManager.GetCollectedCount() >= itemManager.GetTotalCount()) {
            allEvidenceCollected = true;
        }

        // �����t�F�[�Y�ւ̈ڍs����
        if (remainingTime <= 0.0f || allEvidenceCollected) {
            TransitionToReasoning();
        }
    }
    else if (currentPhase == GamePhase::Reasoning) {
        // ���ʕ\����
        if (showResult) {
            resultDisplayTime += delta_second;
            if (resultDisplayTime >= 3.0f) {
                return eSceneType::eTitle;
            }
        }
        else if (reasoningManager) {
            // �����}�l�[�W���[�X�V
            reasoningManager->Update(delta_second);

            // �I���m��`�F�b�N
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
        // �w�i�`��
        if (back_ground_image != -1) {
            DrawGraph(0, 0, back_ground_image, TRUE);
        }

        // ���`��
        g_floor.Draw(cameraX);

        // �A�C�e���`��
        itemManager.Draw(cameraX);

        // �v���C���[�`��
        if (player1) player1->Draw(cameraX);

        // UI�`��
        DrawTimer();
        DrawPhaseInfo();
    }
    else if (currentPhase == GamePhase::Reasoning) {
        // ���ʕ\����
        if (showResult) {
            DrawResult();
        }
        else {
            // ������ʂ̔w�i�i�������j
            if (back_ground_image != -1) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                DrawGraph(0, 0, back_ground_image, TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }

            // ����UI�`��
            if (reasoningUI && reasoningManager) {
                reasoningUI->Draw(reasoningManager);
            }
        }
    }
}

void InGameScene::Finalize()
{
    // BGM��~�E���
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
        printfDx("Map�ǂݍ��݃G���[: %s\n", filename);
        return false;
    }

    std::string line;
    int y = 0;

    while (std::getline(file, line) && y < MAP_HEIGHT) {
        // �J���}��؂�ŉ��
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
        // ���W�����؋������X�g�Ŏ擾
        std::vector<std::string> collectedEvidence = itemManager.GetCollectedItems();

        // �����}�l�[�W���[�ɏ؋���n���ăt�B���^�����O
        reasoningManager->FilterOptions(collectedEvidence);
        reasoningManager->SetActive(true);

        // UI�ɏ؋����X�g��ݒ�
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

    // �w�i�{�b�N�X
    DrawBox(x - 10, y - 10, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);

    // �g��
    DrawBox(x - 10, y - 10, x + boxWidth, y + boxHeight,
        GetColor(100, 100, 100), FALSE);

    // ���ԕ\��
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
    
    DrawFormatString(x, y, GetColor(255, 255, 255), "�؋�: %d/%d", 
        itemManager.GetCollectedCount(), itemManager.GetTotalCount());
}

void InGameScene::DrawOpening() const
{
    int x = 100;
    int y = 100;
    int boxWidth = 1000;
    int boxHeight = 500;

    // �w�i�{�b�N�X
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(255, 255, 255), FALSE);

    DrawFormatString(x, y - 30, GetColor(255, 255, 255), "�I�[�v�j���O");

    int currentY = y + 20;

    // === �����T�v�y�[�W ===
    std::vector<std::string> lines = {
        "�y�ꏊ�z",
        "�E�����s�a�J�� �^�w�O�̋i���X��",
        "",
        "�y���ԁz",
        "�EPM 10:30��",
        "",
        "�y�����T�v�z",
        "��Q�҂͓X���œ|��Ă���Ƃ���𔭌�����܂����B",
        "����ɂ͑�ʂ̌���������A�ꕔ�̏؋��͎U�����Ă��܂����B",
        "",
        "�y�ڌ����z",
        "�E��Q�҂��N���ƌ��������Ă����Ƃ����،�����",
        "�E�X���ɕ����l�𑖂苎��p�������Ƃ̏،�����",
        "",
        "�y���Ȃ��̖ړI�z",
        "- �W�߂��؋�����Ɛl�Ɠ��@�𐄗�����",
        "- �s���R�ȏ،��▵���_��������",
        "- �_���I�ɔƐl����肵�i�荞��",
        "",
        "Z�L�[ �܂��� A�{�^�� �ŊJ�n"
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
    std::string title = isCorrect ? "����!" : "�s����";
    
    DrawFormatString(x + 150, y, titleColor, "%s", title.c_str());
    
    if (isCorrect) {
        DrawFormatString(x + 50, y + 60, GetColor(255, 255, 255), "���߂łƂ��������܂�!");
    }
    else {
        DrawFormatString(x + 50, y + 60, GetColor(255, 255, 255), "�������Ԉ���Ă��܂���");
    }
    
    DrawFormatString(x + 100, y + 120, GetColor(200, 200, 200), "�^�C�g���֖߂�܂�");
}

bool InGameScene::CheckAnswer(const ReasoningOption& selected)
{
    std::string correctSuspect = "�R�c";
    std::string correctMotive = "���ݍ���";
    
    // 安全な文字列比較（空文字列チェック付き）
    if (selected.suspect.empty() || selected.motive.empty()) {
        return false;
    }
    
    return (selected.suspect == correctSuspect && selected.motive == correctMotive);
}
