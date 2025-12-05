#pragma once
#include <vector>
#include <string>
#include <set>

// 推理の要素（犯人、動機、凶器）
struct ReasoningElement {
    std::string text;               // 表示テキスト
    std::string contradictoryEvidence; // この証拠を持っていると「除外」される（消える）
    bool isEliminated;              // 除外フラグ
    bool isCorrect;                 // 正解かどうか

    ReasoningElement(std::string t, std::string contra, bool correct)
        : text(t), contradictoryEvidence(contra), isEliminated(false), isCorrect(correct) {}
};

enum class ReasoningStep {
    Suspect,    // 犯人選択
    Motive,     // 動機選択
    Weapon,     // 凶器選択
    Confirmation // 最終確認
};

class ReasoningManager {
private:
    // 各ステップの選択肢リスト
    std::vector<ReasoningElement> suspects;
    std::vector<ReasoningElement> motives;
    std::vector<ReasoningElement> weapons;

    // 現在の状態
    ReasoningStep currentStep;
    int selectedIndex;
    bool isActive;

    // プレイヤーの選択保持
    int chosenSuspectIndex;
    int chosenMotiveIndex;
    int chosenWeaponIndex;

    // ライフ管理
    int maxLife;
    int currentLife;

public:
    ReasoningManager();
    void Initialize();
    void Update(float deltaTime);
    void Draw() const;

    // 証拠に基づいて選択肢をフィルタリング（消去法）
    void FilterOptions(const std::vector<std::string>& collectedEvidence);

    // 操作
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }

    // ゲッター
    ReasoningStep GetCurrentStep() const { return currentStep; }
    int GetSelectedIndex() const { return selectedIndex; }
    int GetCurrentLife() const { return currentLife; }
    int GetMaxLife() const { return maxLife; }

    // UI描画用データ取得
    const std::vector<ReasoningElement>& GetCurrentOptions() const;
    std::string GetStepTitle() const;
    std::string GetCurrentSelectionText() const; // 現在選んでいるものの確認用

    // 判定
    bool IsGameClear() const;
    bool IsGameOver() const;

private:
    void ConfirmSelection();
    void CheckAnswer();
    void ResetToFirstStep();
};