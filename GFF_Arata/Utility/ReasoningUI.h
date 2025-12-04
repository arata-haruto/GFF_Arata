#pragma once
#include "DxLib.h"
#include "ReasoningManager.h"
#include <vector>
#include <string>

// 推理パート専用UIクラス（ピクセルアート風）
class ReasoningUI {
private:
    // UI要素の位置とサイズ
    static const int WINDOW_X = 80;
    static const int WINDOW_Y = 60;
    static const int WINDOW_WIDTH = 1120;
    static const int WINDOW_HEIGHT = 600;
    
    static const int EVIDENCE_PANEL_X = WINDOW_X + 20;
    static const int EVIDENCE_PANEL_Y = WINDOW_Y + 60;
    static const int EVIDENCE_PANEL_WIDTH = 300;
    static const int EVIDENCE_PANEL_HEIGHT = 500;
    
    static const int OPTION_PANEL_X = EVIDENCE_PANEL_X + EVIDENCE_PANEL_WIDTH + 20;
    static const int OPTION_PANEL_Y = WINDOW_Y + 60;
    static const int OPTION_PANEL_WIDTH = 750;
    static const int OPTION_PANEL_HEIGHT = 500;
    
    static const int OPTION_ITEM_HEIGHT = 50;
    static const int OPTION_SPACING = 5;
    
    // ピクセルアート風の色定義
    unsigned int colorBackground;      // 背景色（暗い茶色）
    unsigned int colorBorder;           // 枠線色（明るい茶色）
    unsigned int colorText;             // 通常テキスト色（白）
    unsigned int colorTextSelected;     // 選択中テキスト色（黄色）
    unsigned int colorTextGrayed;       // グレーアウトテキスト色（灰色）
    unsigned int colorHighlight;        // ハイライト色（青）
    unsigned int colorEvidenceBg;       // 証拠パネル背景色
    unsigned int colorOptionBg;         // 選択肢背景色（通常）
    unsigned int colorOptionBgSelected; // 選択肢背景色（選択中）
    unsigned int colorOptionBgGrayed;   // 選択肢背景色（グレーアウト）
    
    // 証拠リスト
    std::vector<std::string> collectedEvidence;
    
    // ピクセルアート風の描画関数
    void DrawPixelArtBox(int x, int y, int width, int height, 
                         unsigned int fillColor, unsigned int borderColor) const;
    void DrawPixelArtText(int x, int y, unsigned int color, const char* text) const;
    void DrawPixelArtFrame(int x, int y, int width, int height, 
                          unsigned int borderColor) const;
    
public:
    ReasoningUI();
    ~ReasoningUI();
    
    // 初期化
    void Initialize();
    
    // 証拠リストを設定
    void SetEvidenceList(const std::vector<std::string>& evidence);
    
    // 描画（ReasoningManagerの情報を使用）
    void Draw(const ReasoningManager* reasoningManager) const;
    
    // ピクセルアート風の文字列描画（簡易版）
    void DrawPixelString(int x, int y, unsigned int color, const char* format, ...) const;
};

