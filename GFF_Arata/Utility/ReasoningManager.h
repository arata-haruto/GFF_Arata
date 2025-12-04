#pragma once
#include <vector>
#include <string>
#include <set>

// 証拠構造体（タグシステム対応）
struct Evidence {
    std::string name;           // 証拠名
    std::set<std::string> tags; // 証拠タグ（weapon, time, motive, location など）
    
    Evidence() {}
    Evidence(const std::string& n, const std::set<std::string>& t) : name(n), tags(t) {}
};

// 選択肢エフェクト状態
enum class ChoiceEffectState {
    Normal,      // 通常表示
    Flash,       // 白フラッシュ中
    FadeOut,     // フェードアウト中
    Locked       // ロック状態（最終状態）
};

// 選択肢構造体（エフェクト管理拡張）
struct ReasoningOption {
    std::string suspect;
    std::string motive;
    std::set<std::string> requiredEvidence;  // 旧システム（証拠名）
    std::set<std::string> requiredTags;      // 新システム（タグ）
    bool isEliminated;

    // エフェクト管理変数
    ChoiceEffectState effectState;  // エフェクト状態
    float alpha;                    // 透明度（0-255）
    float flashTimer;               // フラッシュタイマー
    float fadeTimer;                // フェードタイマー
    bool wasEnabled;                // 前フレームの有効状態（変化検出用）

    // ★追加：push_back({ "A","B",{...},false }) に対応するコンストラクタ
    ReasoningOption(
        const std::string& s,
        const std::string& m,
        const std::set<std::string>& reqEvidence,
        bool eliminated
    )
        : suspect(s),
        motive(m),
        requiredEvidence(reqEvidence),
        requiredTags(),           // 空のタグ集合
        isEliminated(eliminated),
        effectState(ChoiceEffectState::Normal),
        alpha(255.0f),
        flashTimer(0.0f),
        fadeTimer(0.0f),
        wasEnabled(true)
    {}

    // ★タグ対応版コンストラクタ（必要なら使用）
    ReasoningOption(
        const std::string& s,
        const std::string& m,
        const std::set<std::string>& reqEvidence,
        const std::set<std::string>& reqTags,
        bool eliminated
    )
        : suspect(s),
        motive(m),
        requiredEvidence(reqEvidence),
        requiredTags(reqTags),
        isEliminated(eliminated),
        effectState(ChoiceEffectState::Normal),
        alpha(255.0f),
        flashTimer(0.0f),
        fadeTimer(0.0f),
        wasEnabled(true)
    {}

    // デフォルトコンストラクタ
    ReasoningOption()
        : suspect(""),
        motive(""),
        requiredEvidence(),
        requiredTags(),
        isEliminated(false),
        effectState(ChoiceEffectState::Normal),
        alpha(255.0f),
        flashTimer(0.0f),
        fadeTimer(0.0f),
        wasEnabled(true)
    {}
};
class ReasoningManager {
private:
    std::vector<ReasoningOption> options;
    int selectedIndex;
    bool isActive;
    bool isConfirmed;
    
    // 収集された証拠リスト（タグ情報付き）
    std::vector<Evidence> collectedEvidenceList;
    
    // エフェクト更新用の時間管理
    float deltaTimeAccumulator;

public:
    ReasoningManager();
    void Initialize();
    void Update(float deltaTime = 0.016f);  // deltaTime を追加
    void Draw() const;
    void FilterOptions(const std::vector<std::string>& collectedEvidence);
    
    // 新システム：タグベースのフィルタリング
    void FilterOptionsByTags(const std::vector<Evidence>& evidenceList);
    
    bool IsComplete() const;
    const ReasoningOption& GetSelectedOption() const;
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }
    bool IsConfirmed() const { return isConfirmed; }
    void ResetConfirmed() { isConfirmed = false; }
    
    // 全選択肢を取得（UI描画用）
    const std::vector<ReasoningOption>& GetAllOptions() const { return options; }
    
    // 現在選択中のインデックスを取得
    int GetSelectedIndex() const { return selectedIndex; }
    
    // 選択肢の総数を取得
    size_t GetOptionCount() const { return options.size(); }
    
    // タグ照合関数
    bool CheckChoiceEnabled(const ReasoningOption& option, const std::set<std::string>& collectedTags) const;
    
    // 選択肢エフェクト更新
    void UpdateChoiceEffects(float deltaTime);
};

