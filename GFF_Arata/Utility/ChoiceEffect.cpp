#include "ChoiceEffect.h"
// ★修正: ここで ReasoningManager.h をインクルードして、ReasoningOption の定義を取り込む
#include "ReasoningManager.h" 
#include "DxLib.h"
#include <algorithm>
#include <cmath>

// Windowsのmin/maxマクロとの競合を回避
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// エフェクト定数の定義
const float ChoiceEffectManager::FLASH_DURATION = 0.3f;      // 0.3秒
const float ChoiceEffectManager::FADE_DURATION = 0.8f;        // 0.8秒
const int ChoiceEffectManager::FLASH_ALPHA_MAX = 255;
const int ChoiceEffectManager::FADE_ALPHA_MIN = 80;

// タグ照合関数：選択肢が有効かどうかを判定
bool ChoiceEffectManager::CheckChoiceEnabled(
    const ReasoningOption& option,
    const std::set<std::string>& collectedTags
) {
    // requiredTagsが空の場合は、requiredEvidenceベースの判定にフォールバック
    if (option.requiredTags.empty()) {
        // 旧システム：requiredEvidenceが空の場合は有効とみなす
        return option.requiredEvidence.empty();
    }

    // 必要なタグが全て収集されているかチェック
    for (const auto& requiredTag : option.requiredTags) {
        if (collectedTags.find(requiredTag) == collectedTags.end()) {
            return false;  // 必要なタグが不足
        }
    }
    return true;  // 全てのタグが揃っている
}

// 選択肢エフェクトを更新
void ChoiceEffectManager::UpdateChoiceEffect(ReasoningOption& option, float deltaTime) {
    bool isEnabled = !option.isEliminated;

    // 状態変化を検出（有効→無効）
    if (option.wasEnabled && !isEnabled) {
        // 無効化された瞬間：フラッシュ開始
        option.effectState = ChoiceEffectState::Flash;
        option.flashTimer = 0.0f;
        option.fadeTimer = 0.0f;
        option.alpha = 255.0f;
    }

    option.wasEnabled = isEnabled;

    // エフェクト状態に応じて更新
    switch (option.effectState) {
    case ChoiceEffectState::Normal:
        // 通常状態：有効な選択肢
        option.alpha = 255.0f;
        break;

    case ChoiceEffectState::Flash:
        // 白フラッシュ中
        option.flashTimer += deltaTime;

        if (option.flashTimer >= FLASH_DURATION) {
            // フラッシュ終了 → フェードアウト開始
            option.effectState = ChoiceEffectState::FadeOut;
            option.fadeTimer = 0.0f;
            option.alpha = 255.0f;
        }
        else {
            // フラッシュ効果（白く光る）
            float flashProgress = option.flashTimer / FLASH_DURATION;
            int flashAlpha = (int)(FLASH_ALPHA_MAX * (1.0f - flashProgress));
            option.alpha = 255.0f;  // 背景は白く光る（描画時に処理）
        }
        break;

    case ChoiceEffectState::FadeOut:
        // フェードアウト中
        option.fadeTimer += deltaTime;

        if (option.fadeTimer >= FADE_DURATION) {
            // フェード終了 → ロック状態
            option.effectState = ChoiceEffectState::Locked;
            option.alpha = (float)FADE_ALPHA_MIN;
        }
        else {
            // 255 → 80 まで線形補間
            float fadeProgress = option.fadeTimer / FADE_DURATION;
            option.alpha = 255.0f - (255.0f - FADE_ALPHA_MIN) * fadeProgress;
        }
        break;

    case ChoiceEffectState::Locked:
        // ロック状態（最終状態）
        option.alpha = (float)FADE_ALPHA_MIN;
        break;
    }
}

// 選択肢を描画（エフェクト対応）
void ChoiceEffectManager::DrawChoice(
    int x, int y, int width, int height,
    const ReasoningOption& option,
    bool isSelected,
    int lockIconHandle
) {
    // アルファ値を整数に変換
    int alpha = (int)option.alpha;
    alpha = std::max(0, std::min(255, alpha));

    // 背景色を決定
    unsigned int bgColor;
    if (option.effectState == ChoiceEffectState::Locked) {
        bgColor = GetColor(35, 35, 35);  // グレーアウト背景
    }
    else if (isSelected && option.effectState == ChoiceEffectState::Normal) {
        bgColor = GetColor(100, 80, 60);  // 選択中背景
    }
    else {
        bgColor = GetColor(60, 50, 45);   // 通常背景
    }

    // フラッシュ効果の描画
    if (option.effectState == ChoiceEffectState::Flash) {
        float flashProgress = option.flashTimer / FLASH_DURATION;
        int flashIntensity = (int)(255.0f * (1.0f - flashProgress));

        // 白いフラッシュを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashIntensity);
        DrawBox(x, y, x + width, y + height, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 背景を描画（アルファ適用）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawBox(x, y, x + width, y + height, bgColor, TRUE);

    // 枠線を描画
    if (isSelected && option.effectState == ChoiceEffectState::Normal) {
        DrawBox(x, y, x + width, y + height, GetColor(80, 120, 200), FALSE);
    }
    else if (option.effectState == ChoiceEffectState::Locked) {
        DrawBox(x, y, x + width, y + height, GetColor(100, 100, 100), FALSE);
    }

    // テキスト色を決定
    unsigned int textColor;
    if (option.effectState == ChoiceEffectState::Locked) {
        textColor = GetColor(100, 100, 100);  // グレーアウト
    }
    else if (isSelected) {
        textColor = GetColor(255, 240, 100);  // 選択中（黄色）
    }
    else {
        textColor = GetColor(255, 255, 255);  // 通常（白）
    }

    // テキストを描画（アルファ適用）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // 選択マーカー
    if (isSelected && option.effectState == ChoiceEffectState::Normal) {
        DrawFormatString(x + 5, y + 15, textColor, ">");
    }

    // 選択肢テキスト（安全にアクセス）
    // ★ReasoningManager.h がインクルードされていれば ReasoningOption のメンバが見える
    char buffer[256];
    // ReasoningOptionの構造が変わったため、ReasoningElementのメンバ名に合わせる必要があるかも？
    // しかし、ChoiceEffectは古いReasoningOption構造体を想定している可能性がある。
    // ReasoningManager.hの最新版では `struct ReasoningElement` を使っているが、
    // エラーが出ている ChoiceEffect は `ReasoningOption` を使おうとしている。

    // ※注意: ReasoningManager.h の最新コードでは `ReasoningOption` が `ReasoningElement` に置き換わっているか、
    // あるいは `ReasoningOption` が残っているか確認が必要です。
    // 直前の ReasoningManager.cpp/h の修正で ReasoningOption ではなく ReasoningElement を使うように変更しましたか？
    // もし変更したなら、ChoiceEffect.h/cpp も ReasoningElement を使うように書き換える必要があります。

    // 今回のエラーを見る限り、ChoiceEffect.h はまだ "ReasoningOption" を参照しています。
    // ReasoningManager.h に "ReasoningOption" の定義が残っていればこの修正で通ります。

    // 安全のため、ここでのアクセスは既存のコード通り記述します。
    // もし ReasoningOption が削除されている場合は、ReasoningManager.h に戻すか、ChoiceEffect を ReasoningElement 対応にする必要があります。

    // とりあえず既存コード準拠で出力します：
    const char* suspectStr = option.suspect.empty() ? "(不明)" : option.suspect.c_str();
    const char* motiveStr = option.motive.empty() ? "(不明)" : option.motive.c_str();

    if (option.effectState == ChoiceEffectState::Locked) {
        sprintf_s(buffer, sizeof(buffer), "%s - %s [除外]", suspectStr, motiveStr);
    }
    else {
        sprintf_s(buffer, sizeof(buffer), "%s - %s", suspectStr, motiveStr);
    }

    DrawFormatString(x + (isSelected ? 25 : 20), y + 15, textColor, "%s", buffer);

    // 打ち消し線（ロック状態の場合）
    if (option.effectState == ChoiceEffectState::Locked) {
        int lineY = y + height / 2;
        DrawLine(x + 20, lineY, x + width - 20, lineY, textColor);
    }

    // ロックアイコンを描画（ロック状態の場合）
    if (option.effectState == ChoiceEffectState::Locked) {
        int iconX = x + width - 40;
        int iconY = y + 10;
        DrawLockIcon(iconX, iconY, 20, textColor);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// ロックアイコンの描画（簡易版：DxLibの図形で描画）
void ChoiceEffectManager::DrawLockIcon(int x, int y, int size, unsigned int color) {
    // ロックアイコンを簡易的に描画（四角と円の組み合わせ）
    int halfSize = size / 2;

    // ロック本体（四角）
    DrawBox(x + halfSize / 2, y + halfSize, x + halfSize * 3 / 2, y + size, color, FALSE);

    // ロック上部（半円）
    DrawCircle(x + halfSize, y + halfSize, halfSize / 2, color, FALSE);

    // 鍵穴（小さな円）
    DrawCircle(x + halfSize, y + halfSize * 3 / 2, 2, color, TRUE);
}