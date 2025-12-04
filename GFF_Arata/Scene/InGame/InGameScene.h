#pragma once
#include "../SceneBase.h"
#include <vector>
#include "../../Utility/ReasoningManager.h"
#include "../../Utility/ReasoningUI.h"
//#include "../../Objects/GameObjectManager.h"

class GameObjectManager;
class Player;
class Enemy;

enum class GameState {
	Initializing,
	Result, 
};

enum class GamePhase {
	Opening,
	EvidenceCollection,
	Reasoning
};

class InGameScene : public SceneBase
{
private:
	class GameObjectManager* objm;
	
	Player* player1;
	Player* player2;

	
	int num_image;
	int num_time;
	int back_ground_image = -1;
	
	int mainbgm = -1;

	
	GameState currentGameState;
	GamePhase currentPhase;
	float timeLimit;
	float remainingTime;
	bool allEvidenceCollected;
	class ReasoningManager* reasoningManager;
	class ReasoningUI* reasoningUI;  // 推理パート専用UI
	bool showResult;
	bool isCorrect;
	float resultDisplayTime;
	mutable float cameraX;  // カメラのX座標

public:
	InGameScene();
	virtual void Initialize() override;
	eSceneType Update(float delta_second);
	//virtual eSceneType Update(float delta_second) override;
	virtual void Draw() const override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;

private:
	void TransitionToReasoning();
	void DrawTimer() const;
	void DrawPhaseInfo() const;
	void DrawOpening() const;
	void DrawResult() const;
	bool CheckAnswer(const ReasoningOption& selected);
};
