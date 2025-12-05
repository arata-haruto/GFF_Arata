#include "InGameScene.h"
#include "../SceneFactory.h"
#include "../../Utility/InputManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/Vector2D.h"
#include "../../Objects/Player.h"
#include "../../Objects/Floor.h"
#include "../../Objects/Item.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

// 定数定義
const float SCREEN_WIDTH = 1280.0f;
const float SCREEN_HEIGHT = 720.0f;
const float WORLD_WIDTH = 8000.0f; // 広大なマップ

Floor g_floor;

// コンストラクタ
InGameScene::InGameScene()
	: currentPhase(GamePhase::Opening),
	timeLimit(60.0f),
	remainingTime(60.0f),
	allEvidenceCollected(false),
	reasoningManager(nullptr),
	reasoningUI(nullptr),
	showResult(false),
	isCorrect(false),
	isGameOver(false),
	resultDisplayTime(0.0f),
	cameraX(0.0f),
	player1(nullptr),
	mg_barPosition(0.0f),
	mg_barSpeed(100.0f),
	mg_targetItem(nullptr),
	mg_resultTimer(0.0f),
	mg_lastResultSuccess(false),
	currentAreaIndex(0),
	mainbgm(-1),
	se_success(-1),
	se_fail(-1)
{
	for (int i = 0; i < 4; i++) bgHandles[i] = -1;
}

InGameScene::~InGameScene()
{
	Finalize();
}

void InGameScene::Initialize()
{
	player1 = new Player();
	player1->Initialize();

	g_floor.Initialize();

	// --- アイテム配置（エリア全体に分散）---
	itemManager.Init();
	// Area 1: 商店街
	itemManager.Add(new Item(600, 500, "謎のメモ", "『1分で片付ける』と書かれている。"));
	// Area 2: 公園
	itemManager.Add(new Item(2500, 500, "レシート", "事件時刻に近い時間のレシートだ。"));
	itemManager.Add(new Item(3000, 500, "目撃証言A", "「田中さんはあの時、別の場所にいたよ」"));
	// Area 3: 裏路地
	itemManager.Add(new Item(4500, 500, "防犯カメラ映像", "鈴木とは別の人物が映っている。"));
	// Area 4: 倉庫街
	itemManager.Add(new Item(6500, 500, "検死報告書", "死因は絞殺のようだ。ナイフ傷はない。"));

	// --- 推理システム初期化 ---
	reasoningManager = new ReasoningManager();
	reasoningManager->Initialize();
	reasoningManager->SetActive(false);

	reasoningUI = new ReasoningUI();
	reasoningUI->Initialize();

	currentPhase = GamePhase::Opening;
	remainingTime = timeLimit;
	allEvidenceCollected = false;
	cameraX = 0.0f;

	// --- 背景読み込み ---
	// ※本来は別々の画像を読み込むが、今は同じ画像を流用し、色味で変化をつける
	bgHandles[0] = LoadGraph("Resource/Background/BG.jpg"); // 商店街
	bgHandles[1] = LoadGraph("Resource/Background/BG.jpg"); // 公園（後で差し替え推奨）
	bgHandles[2] = LoadGraph("Resource/Background/BG.jpg"); // 裏路地
	bgHandles[3] = LoadGraph("Resource/Background/BG.jpg"); // 倉庫

	mainbgm = LoadSoundMem("Resource/Sound/BGM.mp3");
	se_success = LoadSoundMem("Resource/Sound/GetItem.mp3");
	se_fail = LoadSoundMem("Resource/Sound/footSE.mp3");

	if (mainbgm != -1) {
		ChangeVolumeSoundMem(180, mainbgm);
		PlaySoundMem(mainbgm, DX_PLAYTYPE_LOOP);
	}
}

eSceneType InGameScene::Update(float delta_second)
{
	InputManager* input = InputManager::GetInstance();

	switch (currentPhase)
	{
	case GamePhase::Opening:
		if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
			input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed)
		{
			currentPhase = GamePhase::EvidenceCollection;
		}
		break;

	case GamePhase::EvidenceCollection:
		if (player1) player1->Update();

		// エリア更新
		UpdateBackground();

		// アイテム管理更新
		itemManager.Update(player1->GetX(), player1->GetY(), delta_second);

		// Eキーでリスト表示
		if (input->GetKeyState(KEY_INPUT_E) == eInputState::Pressed) {
			itemManager.ToggleList();
		}

		// カメラ追従
		{
			float playerX = player1->GetX();
			float targetCameraX = playerX - SCREEN_WIDTH / 2.0f;
			if (targetCameraX < 0.0f) targetCameraX = 0.0f;
			if (targetCameraX > WORLD_WIDTH - SCREEN_WIDTH) targetCameraX = WORLD_WIDTH - SCREEN_WIDTH;
			cameraX += (targetCameraX - cameraX) * 5.0f * delta_second;
		}

		// ミニゲーム開始判定
		if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
			input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed)
		{
			const std::vector<Item*>& items = itemManager.GetItems();
			for (Item* item : items) {
				if (item == nullptr) continue;
				if (!item->GetIsCollected()) {
					float dx = std::abs(player1->GetX() - item->GetX());
					float dy = std::abs(player1->GetY() - item->GetY());
					if (dx < 80.0f && dy < 150.0f) {
						StartMiniGame(item);
						break;
					}
				}
			}
		}

		remainingTime -= delta_second;
		if (itemManager.GetCollectedCount() >= itemManager.GetTotalCount()) {
			allEvidenceCollected = true;
		}
		if (remainingTime <= 0.0f || allEvidenceCollected) {
			remainingTime = 0.0f;
			TransitionToReasoning();
		}
		break;

	case GamePhase::MiniGame:
		UpdateMiniGame(delta_second);
		remainingTime -= delta_second;
		if (remainingTime <= 0.0f) {
			remainingTime = 0.0f;
			TransitionToReasoning();
		}
		break;

	case GamePhase::Reasoning:
		if (showResult) {
			// 結果表示フェーズ
			resultDisplayTime += delta_second;
			if (resultDisplayTime >= 5.0f) {
				return eSceneType::eTitle;
			}
		}
		else if (reasoningManager) {
			// 推理操作（入力処理や判定はReasoningManager内部で行われる）
			reasoningManager->Update(delta_second);

			// ライフ0でゲームオーバー判定
			if (reasoningManager->IsGameOver()) {
				isCorrect = false;
				isGameOver = true;
				showResult = true;
				resultDisplayTime = 0.0f;
				// 失敗SE
				if (se_fail != -1) PlaySoundMem(se_fail, DX_PLAYTYPE_BACK);
			}
			// ゲームクリア判定
			else if (reasoningManager->IsGameClear()) {
				isCorrect = true;
				isGameOver = false;
				showResult = true;
				resultDisplayTime = 0.0f;
				// 成功SE
				if (se_success != -1) PlaySoundMem(se_success, DX_PLAYTYPE_BACK);
			}
		}
		break;
	}

	return GetNowSceneType();
}

void InGameScene::UpdateBackground() {
	if (player1) {
		float x = player1->GetX();
		// 8000のマップを4分割 (2000ずつ)
		if (x < 2000) currentAreaIndex = 0; // 商店街
		else if (x < 4000) currentAreaIndex = 1; // 公園
		else if (x < 6000) currentAreaIndex = 2; // 裏路地
		else currentAreaIndex = 3; // 倉庫
	}
}

void InGameScene::Draw() const
{
	// 背景描画（エリアごとに色を変えて「場所が変わった感」を出す）
	int handle = bgHandles[currentAreaIndex];
	if (handle != -1) {
		// 簡易的な色調補正でエリアを区別
		if (currentAreaIndex == 0) SetDrawBright(255, 255, 255); // 通常
		else if (currentAreaIndex == 1) SetDrawBright(200, 255, 200); // 緑っぽい（公園）
		else if (currentAreaIndex == 2) SetDrawBright(150, 150, 255); // 青っぽい（裏路地）
		else if (currentAreaIndex == 3) SetDrawBright(100, 100, 100); // 暗い（倉庫）

		DrawGraph(0, 0, handle, TRUE);
		SetDrawBright(255, 255, 255); // 戻す
	}

	if (currentPhase == GamePhase::Opening) {
		DrawOpening();
	}
	else if (currentPhase == GamePhase::EvidenceCollection || currentPhase == GamePhase::MiniGame) {
		g_floor.Draw(cameraX);
		itemManager.Draw(cameraX);
		if (player1) player1->Draw(cameraX);
		DrawTimer();
		DrawPhaseInfo();

		// 現在のエリア名表示
		const char* areaNames[] = { "Area 1: 商店街", "Area 2: 公園", "Area 3: 裏路地", "Area 4: 倉庫街" };
		DrawFormatString(1100, 20, GetColor(200, 255, 255), "%s", areaNames[currentAreaIndex]);

		if (currentPhase == GamePhase::MiniGame) {
			DrawMiniGame();
		}
	}
	else if (currentPhase == GamePhase::Reasoning) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200); // 濃い暗転
		DrawBox(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		if (showResult) {
			DrawResult();
		}
		else if (reasoningUI && reasoningManager) {
			reasoningUI->Draw(reasoningManager);
		}
	}
}

void InGameScene::DrawOpening() const
{
	// 背景を少し暗く
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ノベルゲーム風UI
	// 立ち絵スペース（左側）
	DrawBox(100, 100, 400, 600, GetColor(50, 50, 50), TRUE);
	DrawFormatString(180, 300, GetColor(255, 255, 255), "【立ち絵スペース】");
	DrawFormatString(180, 330, GetColor(255, 255, 255), "主人公 or 警察");

	// メッセージウィンドウ（下部）
	int msgX = 50;
	int msgY = 500;
	int msgW = 1180;
	int msgH = 200;

	// ウィンドウ枠
	DrawBox(msgX, msgY, msgX + msgW, msgY + msgH, GetColor(0, 0, 0), TRUE); // 背景
	DrawBox(msgX, msgY, msgX + msgW, msgY + msgH, GetColor(255, 255, 255), FALSE); // 枠線

	// 名前枠
	DrawBox(msgX + 20, msgY - 30, msgX + 200, msgY, GetColor(0, 0, 0), TRUE);
	DrawBox(msgX + 20, msgY - 30, msgX + 200, msgY, GetColor(255, 255, 255), FALSE);
	DrawFormatString(msgX + 40, msgY - 25, GetColor(255, 255, 0), "警察官");

	// テキスト
	DrawFormatString(msgX + 40, msgY + 40, GetColor(255, 255, 255), "「探偵ごときが現場をうろつくな！ 邪魔だ！」");
	DrawFormatString(msgX + 40, msgY + 80, GetColor(255, 255, 255), "「...え？ 1分で解決するだと？ 面白い、やってみろ」");

	DrawFormatString(msgX + 900, msgY + 150, GetColor(200, 200, 200), "Zキーで開始 ▼");
}

void InGameScene::Finalize()
{
	if (mainbgm != -1) { StopSoundMem(mainbgm); DeleteSoundMem(mainbgm); mainbgm = -1; }
	if (se_success != -1) { DeleteSoundMem(se_success); se_success = -1; }
	if (se_fail != -1) { DeleteSoundMem(se_fail); se_fail = -1; }

	// 背景画像の削除
	for (int i = 0; i < 4; i++) {
		if (bgHandles[i] != -1) {
			DeleteGraph(bgHandles[i]);
			bgHandles[i] = -1;
		}
	}

	delete player1; player1 = nullptr;
	delete reasoningManager; reasoningManager = nullptr;
	delete reasoningUI; reasoningUI = nullptr;
}

// === 未解決だった関数の実装 ===

eSceneType InGameScene::GetNowSceneType() const
{
	return eSceneType::eInGame;
}

void InGameScene::TransitionToReasoning()
{
	if (player1) player1->StopAudio();

	currentPhase = GamePhase::Reasoning;
	if (reasoningManager) {
		std::vector<std::string> collected = itemManager.GetCollectedItems();
		reasoningManager->FilterOptions(collected);
		reasoningManager->SetActive(true);
	}
}

void InGameScene::StartMiniGame(Item* item)
{
	currentPhase = GamePhase::MiniGame;
	mg_targetItem = item;
	mg_barPosition = 0.0f;
	mg_barSpeed = 200.0f;
	mg_targetMin = 40.0f;
	mg_targetMax = 60.0f;
	mg_resultTimer = 0.0f;
}

void InGameScene::UpdateMiniGame(float delta_second)
{
	if (mg_resultTimer > 0.0f) {
		mg_resultTimer -= delta_second;
		if (mg_resultTimer <= 0.0f) {
			currentPhase = GamePhase::EvidenceCollection;
		}
		return;
	}

	InputManager* input = InputManager::GetInstance();
	mg_barPosition += mg_barSpeed * delta_second;
	if (mg_barPosition > 100.0f) {
		mg_barPosition = 0.0f;
	}

	if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
		input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed)
	{
		if (mg_barPosition >= mg_targetMin && mg_barPosition <= mg_targetMax) {
			if (se_success != -1) PlaySoundMem(se_success, DX_PLAYTYPE_BACK);
			if (mg_targetItem) {
				mg_targetItem->SetCollected(true);
			}
			mg_lastResultSuccess = true;
		}
		else {
			if (se_fail != -1) PlaySoundMem(se_fail, DX_PLAYTYPE_BACK);
			remainingTime -= 10.0f;
			mg_lastResultSuccess = false;
		}
		mg_resultTimer = 1.0f;
	}
}

void InGameScene::DrawMiniGame() const
{
	int cx = (int)SCREEN_WIDTH / 2;
	int cy = (int)SCREEN_HEIGHT / 2;
	int w = 400;
	int h = 60;

	DrawBox(cx - w / 2 - 5, cy - h / 2 - 5, cx + w / 2 + 5, cy + h / 2 + 5, GetColor(255, 255, 255), FALSE);
	DrawBox(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2, GetColor(30, 30, 30), TRUE);

	if (mg_resultTimer > 0.0f) {
		if (mg_lastResultSuccess) {
			DrawFormatString(cx - 60, cy - 10, GetColor(0, 255, 0), "GET EVIDENCE!");
		}
		else {
			DrawFormatString(cx - 40, cy - 10, GetColor(255, 0, 0), "FAILED...");
			DrawFormatString(cx - 50, cy + 20, GetColor(255, 100, 100), "-10 Seconds");
		}
		return;
	}

	if (mg_targetItem) {
		DrawFormatString(cx - 100, cy - 60, GetColor(255, 255, 0), "TARGET: %s", mg_targetItem->GetName().c_str());
	}

	int z1 = cx - w / 2 + (int)(w * (mg_targetMin / 100.0f));
	int z2 = cx - w / 2 + (int)(w * (mg_targetMax / 100.0f));
	DrawBox(z1, cy - h / 2, z2, cy + h / 2, GetColor(0, 255, 0), TRUE);

	int bx = cx - w / 2 + (int)(w * (mg_barPosition / 100.0f));
	DrawBox(bx - 3, cy - h / 2 - 5, bx + 3, cy + h / 2 + 5, GetColor(255, 50, 50), TRUE);

	DrawFormatString(cx - 80, cy + 40, GetColor(255, 255, 255), "Zキーでタイミングよく止めろ！");
}

void InGameScene::DrawTimer() const
{
	int min = (int)remainingTime / 60;
	int sec = (int)remainingTime % 60;
	int msec = (int)((remainingTime - (int)remainingTime) * 100);
	unsigned int color = GetColor(255, 255, 255);
	if (remainingTime < 10.0f) color = GetColor(255, 0, 0);
	DrawFormatString(20, 20, color, "TIME: %02d:%02d.%02d", min, sec, msec);
}

void InGameScene::DrawPhaseInfo() const
{
	DrawFormatString(20, 50, GetColor(200, 200, 200), "証拠発見数: %d / %d",
		itemManager.GetCollectedCount(), itemManager.GetTotalCount());
}

void InGameScene::DrawResult() const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
	DrawBox(300, 200, (int)SCREEN_WIDTH - 300, (int)SCREEN_HEIGHT - 200, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	if (isCorrect) {
		DrawFormatString(550, 300, GetColor(0, 255, 0), "推理 正解！！");
		DrawFormatString(500, 350, GetColor(255, 255, 255), "見事、1分で事件を解決した！");
	}
	else {
		DrawFormatString(550, 300, GetColor(255, 0, 0), "推理 失敗...");
		DrawFormatString(500, 350, GetColor(255, 255, 255), "真実は闇の中へ...");
	}
}