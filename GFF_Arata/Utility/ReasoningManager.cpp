#include "ReasoningManager.h"
#include "DxLib.h"
#include "InputManager.h"
#include "ChoiceEffect.h"
#include <algorithm>

ReasoningManager::ReasoningManager()
    : selectedIndex(0),
      isActive(false),
      isConfirmed(false),
      deltaTimeAccumulator(0.0f)
{
}

void ReasoningManager::Initialize() {
    options.clear();
    selectedIndex = 0;
    collectedEvidenceList.clear();
    deltaTimeAccumulator = 0.0f;

    // サンプルの選択肢データ（日本語は UTF-8 で保存してください）
    options.push_back(ReasoningOption("田中", "復讐心による",
        std::set<std::string>{ "証拠1", "証拠2" },
        std::set<std::string>{ "weapon", "time" }, false));

    options.push_back(ReasoningOption("佐藤", "嫉妬",
        std::set<std::string>{ "証拠3", "証拠2" },
        std::set<std::string>{ "motive", "location" }, false));

    options.push_back(ReasoningOption("鈴木", "金銭",
        std::set<std::string>{ "証拠1" },
        std::set<std::string>{ "weapon" }, false));

    options.push_back(ReasoningOption("山田", "過去の因縁",
        std::set<std::string>{ "証拠2" },
        std::set<std::string>{ "time" }, false));

    options.push_back(ReasoningOption("高橋", "遺産相続",
        std::set<std::string>{ "証拠3", "証拠1" },
        std::set<std::string>{ "motive", "weapon" }, false));

    options.push_back(ReasoningOption("伊藤", "恨みの連鎖",
        std::set<std::string>{ "証拠2", "証拠3" },
        std::set<std::string>{ "time", "location" }, false));

    options.push_back(ReasoningOption("渡辺", "秘密の暴露",
        std::set<std::string>{ "証拠1" },
        std::set<std::string>{ "weapon" }, false));

    options.push_back(ReasoningOption("中村", "証拠",
        std::set<std::string>{ "証拠2" },
        std::set<std::string>{ "time" }, false));

    options.push_back(ReasoningOption("小林", "誤解",
        std::set<std::string>{ "証拠3" },
        std::set<std::string>{ "location" }, false));

    options.push_back(ReasoningOption("加藤", "複雑な関係性",
        std::set<std::string>{ "証拠1", "証拠2", "証拠3" },
        std::set<std::string>{ "weapon", "time", "location" }, false));
}

void ReasoningManager::Update(float deltaTime) {
    if (!isActive) return;

    // エフェクト更新
    UpdateChoiceEffects(deltaTime);

    InputManager* input = InputManager::GetInstance();

    // 上移動
    if (input->GetKeyState(KEY_INPUT_UP) == eInputState::Pressed ||
        input->GetButtonState(XINPUT_BUTTON_DPAD_UP) == eInputState::Pressed) {
        int newIndex = selectedIndex;
        do {
            newIndex--;
            if (newIndex < 0) {
                // 末尾の有効な要素へラップ
                for (int i = (int)options.size() - 1; i >= 0; i--) {
                    if (!options[i].isEliminated) {
                        newIndex = i;
                        break;
                    }
                }
                break;
            }
        } while (newIndex >= 0 && options[newIndex].isEliminated);

        if (newIndex >= 0 && !options[newIndex].isEliminated) {
            selectedIndex = newIndex;
        }
    }

    // 下移動
    if (input->GetKeyState(KEY_INPUT_DOWN) == eInputState::Pressed ||
        input->GetButtonState(XINPUT_BUTTON_DPAD_DOWN) == eInputState::Pressed) {
        int newIndex = selectedIndex;
        do {
            newIndex++;
            if (newIndex >= (int)options.size()) {
                // 先頭の有効な要素へラップ
                for (size_t i = 0; i < options.size(); i++) {
                    if (!options[i].isEliminated) {
                        newIndex = (int)i;
                        break;
                    }
                }
                break;
            }
        } while (newIndex < (int)options.size() && options[newIndex].isEliminated);

        if (newIndex < (int)options.size() && !options[newIndex].isEliminated) {
            selectedIndex = newIndex;
        }
    }

    // 決定
    if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
        input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed) {
        if (selectedIndex >= 0 && selectedIndex < (int)options.size() &&
            !options[selectedIndex].isEliminated) {
            isConfirmed = true;
        }
    }
}

void ReasoningManager::Draw() const {
    if (!isActive) return;

    int x = 100;
    int y = 100;
    int boxWidth = 600;
    int boxHeight = 500;

    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(0, 0, 0), TRUE);
    DrawBox(x - 20, y - 40, x + boxWidth, y + boxHeight,
        GetColor(255, 255, 255), FALSE);

    DrawFormatString(x, y - 30, GetColor(255, 255, 255), "推理 - 候補一覧");

    int drawY = y;

    for (size_t i = 0; i < options.size(); i++) {
        if (options[i].isEliminated) continue;

        bool isSelected = ((int)i == selectedIndex);

        // ChoiceEffectManager に描画を任せる（必要ならカスタム描画）
        ChoiceEffectManager::DrawChoice(x, drawY, boxWidth, 36, options[i], isSelected, -1);

        drawY += 40;
    }

    DrawFormatString(x, drawY + 20, GetColor(200, 200, 200), "Zキーで決定");
}

void ReasoningManager::FilterOptions(const std::vector<std::string>& collectedEvidence) {
    std::set<std::string> evidenceSet;
    for (const auto& ev : collectedEvidence) {
        evidenceSet.insert(ev);
    }

    for (auto& option : options) {
        bool hasAllEvidence = true;
        for (const auto& required : option.requiredEvidence) {
            if (evidenceSet.find(required) == evidenceSet.end()) {
                hasAllEvidence = false;
                break;
            }
        }

        if (!hasAllEvidence) {
            option.isEliminated = true;
        } else {
            option.isEliminated = false;
        }
    }

    // 選択中が除外されていたら最初の有効な選択肢へ
    if (selectedIndex >= 0 && selectedIndex < (int)options.size() && options[selectedIndex].isEliminated) {
        for (size_t i = 0; i < options.size(); i++) {
            if (!options[i].isEliminated) {
                selectedIndex = (int)i;
                break;
            }
        }
    }
}

bool ReasoningManager::IsComplete() const {
    int remainingCount = 0;
    for (const auto& option : options) {
        if (!option.isEliminated) remainingCount++;
    }
    return remainingCount <= 1;
}

const ReasoningOption& ReasoningManager::GetSelectedOption() const {
    // 静的オブジェクトを返す（安全に初期化される）
    static const ReasoningOption emptyOption = ReasoningOption();
    
    // 範囲チェックとベクターの有効性チェック
    if (selectedIndex >= 0 && selectedIndex < (int)options.size() && !options.empty()) {
        return options[selectedIndex];
    }
    return emptyOption;
}

bool ReasoningManager::CheckChoiceEnabled(
    const ReasoningOption& option,
    const std::set<std::string>& collectedTags
) const {
    return ChoiceEffectManager::CheckChoiceEnabled(option, collectedTags);
}

void ReasoningManager::UpdateChoiceEffects(float deltaTime) {
    // エフェクトの更新のみを行う（有効性の判定は FilterOptionsByTags で行う）
    for (auto& option : options) {
        ChoiceEffectManager::UpdateChoiceEffect(option, deltaTime);
    }
}

void ReasoningManager::FilterOptionsByTags(const std::vector<Evidence>& evidenceList) {
    // 収集されたタグを抽出
    std::set<std::string> collectedTags;
    for (const auto& evidence : evidenceList) {
        for (const auto& tag : evidence.tags) {
            collectedTags.insert(tag);
        }
    }

    for (auto& option : options) {
        bool wasEliminated = option.isEliminated;
        bool isEnabled = CheckChoiceEnabled(option, collectedTags);
        option.isEliminated = !isEnabled;

        if (!wasEliminated && option.isEliminated) {
            option.effectState = ChoiceEffectState::Flash;
            option.flashTimer = 0.0f;
            option.fadeTimer = 0.0f;
            option.alpha = 255.0f;
        }
    }

    // 選択中が除外されていたら最初の有効な選択肢へ
    if (selectedIndex >= 0 && selectedIndex < (int)options.size() && options[selectedIndex].isEliminated) {
        for (size_t i = 0; i < options.size(); i++) {
            if (!options[i].isEliminated) {
                selectedIndex = (int)i;
                break;
            }
        }
    }
}
