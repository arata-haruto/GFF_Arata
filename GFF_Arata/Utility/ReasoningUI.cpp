#include "ReasoningUI.h"
#include "DxLib.h"
#include <cstdarg>
#include <cstdio>

ReasoningUI::ReasoningUI() {
    colorBackground = GetColor(30, 30, 35);
    colorBorder = GetColor(200, 200, 200);
    colorText = GetColor(255, 255, 255);
    colorTextSelected = GetColor(255, 255, 0);
    colorHighlight = GetColor(50, 50, 100);
    colorLife = GetColor(255, 50, 50);
}

void ReasoningUI::Initialize() {
    collectedEvidence.clear();
}

void ReasoningUI::SetEvidenceList(const std::vector<std::string>& evidence) {
    collectedEvidence = evidence;
}

void ReasoningUI::DrawPixelArtBox(int x, int y, int width, int height, unsigned int fill, unsigned int border) const {
    DrawBox(x, y, x + width, y + height, fill, TRUE);
    DrawBox(x, y, x + width, y + height, border, FALSE);
    DrawBox(x + 2, y + 2, x + width - 2, y + height - 2, border, FALSE); // 二重線
}

void ReasoningUI::DrawPixelString(int x, int y, unsigned int color, const char* format, ...) const {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    DrawFormatString(x, y, color, "%s", buffer);
}

void ReasoningUI::DrawLife(int current, int max) const {
    int startX = WINDOW_X + WINDOW_WIDTH - 200;
    int y = WINDOW_Y - 40;
    DrawPixelString(startX - 60, y, colorText, "Life:");

    for (int i = 0; i < max; i++) {
        if (i < current) {
            // ハート（塗りつぶし）
            DrawBox(startX + i * 40, y, startX + i * 40 + 30, y + 30, colorLife, TRUE);
        }
        else {
            // ハート（枠のみ）
            DrawBox(startX + i * 40, y, startX + i * 40 + 30, y + 30, GetColor(100, 100, 100), FALSE);
        }
    }
}

void ReasoningUI::Draw(const ReasoningManager* manager) const {
    if (!manager || !manager->IsActive()) return;

    // 背景
    DrawPixelArtBox(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, colorBackground, colorBorder);

    // ヘッダー情報
    DrawPixelString(WINDOW_X + 20, WINDOW_Y + 20, colorText, "【事件概要】");
    DrawPixelString(WINDOW_X + 20, WINDOW_Y + 50, colorText, "深夜の雑貨店で発生した強盗殺人事件。犯人はまだ近くにいる可能性がある。");
    DrawPixelString(WINDOW_X + 20, WINDOW_Y + 80, colorText, "制限時間内に証拠を集め、論理的に犯人を特定せよ。");

    // ライフ表示
    DrawLife(manager->GetCurrentLife(), manager->GetMaxLife());

    // 現在のステップタイトル
    DrawPixelString(WINDOW_X + 40, WINDOW_Y + 130, colorTextSelected, "%s", manager->GetStepTitle().c_str());

    // 現在の選択状況
    DrawPixelString(WINDOW_X + 40, WINDOW_Y + 550, GetColor(200, 200, 255), "現在の推理: %s", manager->GetCurrentSelectionText().c_str());

    // 選択肢の描画
    if (manager->GetCurrentStep() != ReasoningStep::Confirmation) {
        const auto& options = manager->GetCurrentOptions();
        int drawY = WINDOW_Y + 180;
        int idx = 0;

        for (const auto& opt : options) {
            if (opt.isEliminated) {
                // 除外された選択肢は描画しない、または薄く表示
                // ここでは「消える」要望に合わせて描画スキップ（インデックスはずれるので注意が必要だがManager側で制御済み）
                continue;
            }

            bool isSelected = (idx == manager->GetSelectedIndex());
            unsigned int color = isSelected ? colorTextSelected : colorText;

            if (isSelected) {
                DrawBox(WINDOW_X + 40, drawY - 5, WINDOW_X + WINDOW_WIDTH - 40, drawY + 35, colorHighlight, TRUE);
                DrawPixelString(WINDOW_X + 50, drawY, color, "> %s", opt.text.c_str());
            }
            else {
                DrawPixelString(WINDOW_X + 70, drawY, color, "%s", opt.text.c_str());
            }
            drawY += 50;
            idx++;
        }
    }
    else {
        // 確認画面
        int drawY = WINDOW_Y + 250;
        bool isSelectedYes = (manager->GetSelectedIndex() == 0);

        if (isSelectedYes) DrawBox(WINDOW_X + 300, drawY - 5, WINDOW_X + 700, drawY + 35, colorHighlight, TRUE);
        DrawPixelString(WINDOW_X + 450, drawY, isSelectedYes ? colorTextSelected : colorText, "これで推理する！");

        drawY += 60;
        if (!isSelectedYes) DrawBox(WINDOW_X + 300, drawY - 5, WINDOW_X + 700, drawY + 35, colorHighlight, TRUE);
        DrawPixelString(WINDOW_X + 450, drawY, !isSelectedYes ? colorTextSelected : colorText, "考え直す...");
    }
}