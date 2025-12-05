#pragma once
#include "../SceneBase.h"
#include <vector>
#include "../../Utility/ReasoningManager.h"
#include "../../Utility/ReasoningUI.h"
#include "../../Utility/ItemManager.h"

// クラスの前方宣言
class GameObjectManager;
class Player;
class Item;

enum class GamePhase {
	Opening,
	EvidenceCollection,
	MiniGame,
	Reasoning
};

class InGameScene : public SceneBase
{
private:
	Player* player1;
	ItemManager itemManager;

	// --- 背景管理 ---
	// 複数の背景画像をエリアごとに切り替える
	int bgHandles[4]; // 4エリア分
	int currentAreaIndex; // 現在のエリア番号(0-3)

	int mainbgm = -1;
	int se_success = -1;
	int se_fail = -1;

	GamePhase currentPhase;
	float timeLimit;
	float remainingTime;
	bool allEvidenceCollected;

	mutable float cameraX;

	class ReasoningManager* reasoningManager;
	class ReasoningUI* reasoningUI;

	// 結果表示用
	bool showResult;
	bool isCorrect;       // ゲームクリアか
	bool isGameOver;      // ゲームオーバーか
	float resultDisplayTime;

	// ミニゲーム用
	float mg_barPosition;
	float mg_barSpeed;
	float mg_targetMin;
	float mg_targetMax;
	Item* mg_targetItem;
	float mg_resultTimer;
	bool mg_lastResultSuccess;

public:
	InGameScene();
	virtual ~InGameScene();

	virtual void Initialize() override;
	virtual eSceneType Update(float delta_second) override;
	virtual void Draw() const override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;

private:
	void TransitionToReasoning();

	void DrawTimer() const;
	void DrawPhaseInfo() const;
	void DrawOpening() const;       // ノベルゲーム風オープニング
	void DrawResult() const;
	void DrawMiniGame() const;

	void StartMiniGame(Item* item);
	void UpdateMiniGame(float delta_second);

	// 背景更新
	void UpdateBackground();
};