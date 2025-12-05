#include "ReasoningManager.h"
#include "DxLib.h"
#include "InputManager.h"
#include "ChoiceEffect.h"
#include <algorithm>

ReasoningManager::ReasoningManager()
    : currentStep(ReasoningStep::Suspect), selectedIndex(0), isActive(false),
    chosenSuspectIndex(-1), chosenMotiveIndex(-1), chosenWeaponIndex(-1),
    maxLife(3), currentLife(3) // ライフは3（2回ミスOK）
{
}

void ReasoningManager::Initialize() {
    suspects.clear();
    motives.clear();
    weapons.clear();
    currentStep = ReasoningStep::Suspect;
    selectedIndex = 0;
    currentLife = maxLife;

    // === 犯人候補 (テキスト, 否定する証拠, 正解フラグ) ===
    // 証拠「目撃証言A」を持っていると「田中」はシロだとわかり、リストから消える
    suspects.push_back(ReasoningElement("田中（被害者の友人）", "目撃証言A", false));
    suspects.push_back(ReasoningElement("佐藤（店のオーナー）", "レシート", false));
    suspects.push_back(ReasoningElement("山田（元従業員）", "", true)); // 正解：否定証拠なし
    suspects.push_back(ReasoningElement("鈴木（通り魔）", "防犯カメラ映像", false));

    // === 動機候補 ===
    motives.push_back(ReasoningElement("金銭トラブル", "借用書", false));
    motives.push_back(ReasoningElement("過去の因縁", "", true)); // 正解
    motives.push_back(ReasoningElement("突発的な犯行", "計画メモ", false));
    motives.push_back(ReasoningElement("口封じ", "脅迫状", false));

    // === 凶器候補 ===
    weapons.push_back(ReasoningElement("ナイフ", "検死報告書", false));
    weapons.push_back(ReasoningElement("鈍器（置物）", "", true)); // 正解（壊れた時計など）
    weapons.push_back(ReasoningElement("毒物", "現場写真", false));
    weapons.push_back(ReasoningElement("ロープ", "検死報告書", false));
}

void ReasoningManager::Update(float deltaTime) {
    if (!isActive) return;

    InputManager* input = InputManager::GetInstance();
    const auto& options = GetCurrentOptions();

    // 上下移動（除外されている選択肢はスキップ）
    // ★修正: optionsが空の場合は処理しない安全策を追加
    if (!options.empty()) {
        if (input->GetKeyState(KEY_INPUT_UP) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_DPAD_UP) == eInputState::Pressed) {
            int originalIndex = selectedIndex;
            do {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = (int)options.size() - 1;
            } while (options[selectedIndex].isEliminated && selectedIndex != originalIndex);
        }

        if (input->GetKeyState(KEY_INPUT_DOWN) == eInputState::Pressed ||
            input->GetButtonState(XINPUT_BUTTON_DPAD_DOWN) == eInputState::Pressed) {
            int originalIndex = selectedIndex;
            do {
                selectedIndex++;
                if (selectedIndex >= (int)options.size()) selectedIndex = 0;
            } while (options[selectedIndex].isEliminated && selectedIndex != originalIndex);
        }
    }

    // 決定
    if (input->GetKeyState(KEY_INPUT_Z) == eInputState::Pressed ||
        input->GetButtonState(XINPUT_BUTTON_A) == eInputState::Pressed) {
        ConfirmSelection();
    }

    // キャンセル（前のステップに戻る）
    if ((input->GetKeyState(KEY_INPUT_X) == eInputState::Pressed ||
        input->GetButtonState(XINPUT_BUTTON_B) == eInputState::Pressed) && currentStep != ReasoningStep::Suspect) {

        if (currentStep == ReasoningStep::Motive) currentStep = ReasoningStep::Suspect;
        else if (currentStep == ReasoningStep::Weapon) currentStep = ReasoningStep::Motive;
        else if (currentStep == ReasoningStep::Confirmation) currentStep = ReasoningStep::Weapon;

        selectedIndex = 0;
    }
}

void ReasoningManager::ConfirmSelection() {
    if (currentStep == ReasoningStep::Suspect) {
        chosenSuspectIndex = selectedIndex;
        currentStep = ReasoningStep::Motive;
        selectedIndex = 0;
    }
    else if (currentStep == ReasoningStep::Motive) {
        chosenMotiveIndex = selectedIndex;
        currentStep = ReasoningStep::Weapon;
        selectedIndex = 0;
    }
    else if (currentStep == ReasoningStep::Weapon) {
        chosenWeaponIndex = selectedIndex;
        currentStep = ReasoningStep::Confirmation;
        selectedIndex = 0; // Yes/No選択用（0:推理する 1:見直す）
    }
    else if (currentStep == ReasoningStep::Confirmation) {
        if (selectedIndex == 0) {
            CheckAnswer(); // 推理実行！
        }
        else {
            ResetToFirstStep(); // 最初からやり直し
        }
    }

    // 次のステップの選択肢がすべて除外されていた場合の安全策（先頭を探す）
    if (currentStep != ReasoningStep::Confirmation) {
        const auto& options = GetCurrentOptions();
        for (int i = 0; i < options.size(); i++) {
            if (!options[i].isEliminated) {
                selectedIndex = i;
                break;
            }
        }
    }
}

void ReasoningManager::CheckAnswer() {
    bool isSuspectCorrect = suspects[chosenSuspectIndex].isCorrect;
    bool isMotiveCorrect = motives[chosenMotiveIndex].isCorrect;
    bool isWeaponCorrect = weapons[chosenWeaponIndex].isCorrect;

    if (isSuspectCorrect && isMotiveCorrect && isWeaponCorrect) {
        // 完全正解！ -> InGameScene側でクリア検知
    }
    else {
        // 不正解 -> ライフ減少
        currentLife--;
        if (currentLife > 0) {
            // まだライフがあるなら最初から
            ResetToFirstStep();
        }
        else {
            // ゲームオーバー -> InGameScene側で検知
        }
    }
}

void ReasoningManager::ResetToFirstStep() {
    currentStep = ReasoningStep::Suspect;
    selectedIndex = 0;
    // 最初の有効な選択肢へ
    for (int i = 0; i < suspects.size(); i++) {
        if (!suspects[i].isEliminated) {
            selectedIndex = i;
            break;
        }
    }
}

void ReasoningManager::FilterOptions(const std::vector<std::string>& collectedEvidence) {
    auto filter = [&](std::vector<ReasoningElement>& list) {
        for (auto& item : list) {
            if (item.contradictoryEvidence.empty()) continue; // 否定証拠設定なしなら消えない

            // 持っている証拠リストの中に、この選択肢を否定するものがあるか？
            for (const auto& evidence : collectedEvidence) {
                if (evidence == item.contradictoryEvidence) {
                    item.isEliminated = true; // 証拠があるので「シロ」としてリストから消す
                    break;
                }
            }
        }
        };

    filter(suspects);
    filter(motives);
    filter(weapons);
}

const std::vector<ReasoningElement>& ReasoningManager::GetCurrentOptions() const {
    if (currentStep == ReasoningStep::Suspect) return suspects;
    if (currentStep == ReasoningStep::Motive) return motives;
    if (currentStep == ReasoningStep::Weapon) return weapons;

    // ★修正点: 最終確認画面（Confirmation）用に、ダミーの選択肢（2つ）を返すように変更
    // これにより、Update関数内のループ処理で空の配列にアクセスしてクラッシュするのを防ぎます
    static std::vector<ReasoningElement> confirmationDummy = {
        ReasoningElement("Yes", "", false),
        ReasoningElement("No", "", false)
    };
    return confirmationDummy;
}

std::string ReasoningManager::GetStepTitle() const {
    switch (currentStep) {
    case ReasoningStep::Suspect: return "Step 1: 犯人は誰だ？";
    case ReasoningStep::Motive:  return "Step 2: 動機は何だ？";
    case ReasoningStep::Weapon:  return "Step 3: 凶器は何だ？";
    case ReasoningStep::Confirmation: return "最終確認：これで間違いないか？";
    default: return "";
    }
}

std::string ReasoningManager::GetCurrentSelectionText() const {
    std::string s = (chosenSuspectIndex >= 0) ? suspects[chosenSuspectIndex].text : "???";
    std::string m = (chosenMotiveIndex >= 0) ? motives[chosenMotiveIndex].text : "???";
    std::string w = (chosenWeaponIndex >= 0) ? weapons[chosenWeaponIndex].text : "???";
    return "犯人:" + s + " / 動機:" + m + " / 凶器:" + w;
}

bool ReasoningManager::IsGameClear() const {
    // ライフが残っていて、確認フェーズで正解した場合のみここに来るロジックにするか、
    // あるいは外部から呼び出すフラグを持たせるのが良いが、簡易的に
    // ライフ更新後のチェックで判定する
    // 本来はCheckAnswerで正解フラグを立てるべきだが、InGameScene側で管理しているためここでは常にfalseを返す
    // （InGameScene側で管理する場合はここでの実装は不要だが、インターフェースとして残す）
    return false;
}

bool ReasoningManager::IsGameOver() const {
    return currentLife <= 0;
}

// 描画はUIクラスに任せるため空
void ReasoningManager::Draw() const {}