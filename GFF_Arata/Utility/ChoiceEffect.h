#pragma once
#include "DxLib.h"
#include <string>
#include <set>
#include "ReasoningManager.h"

// 選択肢エフェクト管理クラス
class ChoiceEffectManager {
public:
    // エフェクト定数
    static const float FLASH_DURATION;      // フラッシュ時間（秒）
    static const float FADE_DURATION;       // フェードアウト時間（秒）
    static const int FLASH_ALPHA_MAX;       // フラッシュ最大透明度
    static const int FADE_ALPHA_MIN;         // フェード後最小透明度
    
    // 選択肢の有効性をチェック（タグ照合）
    static bool CheckChoiceEnabled(
        const ReasoningOption& option, 
        const std::set<std::string>& collectedTags
    );
    
    // 選択肢エフェクトを更新
    static void UpdateChoiceEffect(ReasoningOption& option, float deltaTime);
    
    // 選択肢を描画（エフェクト対応）
    static void DrawChoice(
        int x, int y, int width, int height,
        const ReasoningOption& option,
        bool isSelected,
        int lockIconHandle = -1
    );
    
    // ロックアイコンの読み込み（簡易版：DxLibの図形で描画）
    static void DrawLockIcon(int x, int y, int size, unsigned int color);
};

