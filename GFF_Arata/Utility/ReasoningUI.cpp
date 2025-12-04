#include "ReasoningUI.h"
#include "DxLib.h"
#include "ChoiceEffect.h"
#include <cstdarg>
#include <cstdio>

ReasoningUI::ReasoningUI() {
    // �s�N�Z���A�[�g���̐F���������i�������[���b�p���̐F���j
    colorBackground = GetColor(40, 30, 25);        // �Â����F�i�w�i�j
    colorBorder = GetColor(180, 150, 120);         // ���邢���F�i�g���j
    colorText = GetColor(255, 255, 255);           // ���i�ʏ�e�L�X�g�j
    colorTextSelected = GetColor(255, 240, 100);   // ���F�i�I�𒆃e�L�X�g�j
    colorTextGrayed = GetColor(100, 100, 100);     // �D�F�i�O���[�A�E�g�j
    colorHighlight = GetColor(80, 120, 200);       // �i�n�C���C�g�j
    colorEvidenceBg = GetColor(50, 40, 35);        // �؋��p�l���w�i
    colorOptionBg = GetColor(60, 50, 45);          // �I�����w�i�i�ʏ�j
    colorOptionBgSelected = GetColor(100, 80, 60); // �I�����w�i�i�I�𒆁j
    colorOptionBgGrayed = GetColor(35, 35, 35);     // �I�����w�i�i�O���[�A�E�g�j
}

ReasoningUI::~ReasoningUI() {
}

void ReasoningUI::Initialize() {
    collectedEvidence.clear();
}

void ReasoningUI::SetEvidenceList(const std::vector<std::string>& evidence) {
    collectedEvidence = evidence;
}

// �s�N�Z���A�[�g���̃{�b�N�X�`��i�h�b�g�����o�����߁A�����e�߂Ɂj
void ReasoningUI::DrawPixelArtBox(int x, int y, int width, int height, 
                                   unsigned int fillColor, unsigned int borderColor) const {
    // �w�i��`��
    DrawBox(x, y, x + width, y + height, fillColor, TRUE);
    
    // �s�N�Z���A�[�g���̘g���i���߂̐��j
    int borderWidth = 3;
    // ��
    DrawBox(x, y, x + width, y + borderWidth, borderColor, TRUE);
    // ��
    DrawBox(x, y + height - borderWidth, x + width, y + height, borderColor, TRUE);
    // ��
    DrawBox(x, y, x + borderWidth, y + height, borderColor, TRUE);
    // �E
    DrawBox(x + width - borderWidth, y, x + width, y + height, borderColor, TRUE);
    
    // �����̉e�i�s�N�Z���A�[�g���̗��̊��j
    unsigned int shadowColor = GetColor(20, 15, 10);
    DrawBox(x + borderWidth, y + borderWidth, x + width - borderWidth, 
            y + borderWidth + 2, shadowColor, TRUE);
    DrawBox(x + borderWidth, y + borderWidth, x + borderWidth + 2, 
            y + height - borderWidth, shadowColor, TRUE);
}

// �s�N�Z���A�[�g���̃t���[���`��
void ReasoningUI::DrawPixelArtFrame(int x, int y, int width, int height, 
                                     unsigned int borderColor) const {
    int borderWidth = 3;
    // �O���̘g
    DrawBox(x, y, x + width, y + borderWidth, borderColor, TRUE);
    DrawBox(x, y + height - borderWidth, x + width, y + height, borderColor, TRUE);
    DrawBox(x, y, x + borderWidth, y + height, borderColor, TRUE);
    DrawBox(x + width - borderWidth, y, x + width, y + height, borderColor, TRUE);
    
    // �����̉e
    unsigned int shadowColor = GetColor(20, 15, 10);
    DrawBox(x + borderWidth, y + borderWidth, x + width - borderWidth, 
            y + borderWidth + 2, shadowColor, TRUE);
    DrawBox(x + borderWidth, y + borderWidth, x + borderWidth + 2, 
            y + height - borderWidth, shadowColor, TRUE);
}

// �s�N�Z���A�[�g���̕�����`��i�t�H���g�T�C�Y�𒲐��j
void ReasoningUI::DrawPixelString(int x, int y, unsigned int color, const char* format, ...) const {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // DxLib�̕W���t�H���g���g�p�i�s�N�Z���A�[�g���ɂ���ꍇ�͐�p�t�H���g��ǂݍ��ށj
    DrawFormatString(x, y, color, "%s", buffer);
}

void ReasoningUI::Draw(const ReasoningManager* reasoningManager) const {
    if (!reasoningManager || !reasoningManager->IsActive()) return;
    
    // ===== ���C���E�B���h�E�i�w�i�j =====
    DrawPixelArtBox(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, 
                    colorBackground, colorBorder);
    
    // �^�C�g��
    DrawPixelString(WINDOW_X + 20, WINDOW_Y + 20, colorTextSelected, "�����t�F�[�Y - �Ɛl�Ɠ��@��I��");
    
    // ===== �����F�؋��p�l�� =====
    DrawPixelArtBox(EVIDENCE_PANEL_X, EVIDENCE_PANEL_Y, 
                    EVIDENCE_PANEL_WIDTH, EVIDENCE_PANEL_HEIGHT,
                    colorEvidenceBg, colorBorder);
    
    DrawPixelString(EVIDENCE_PANEL_X + 10, EVIDENCE_PANEL_Y + 10, colorTextSelected, "���W�����؋�");
    
    // �؋����X�g��\��
    int evidenceY = EVIDENCE_PANEL_Y + 40;
    if (collectedEvidence.empty()) {
        DrawPixelString(EVIDENCE_PANEL_X + 10, evidenceY, colorTextGrayed, "�؋�������܂���");
    } else {
        for (size_t i = 0; i < collectedEvidence.size(); i++) {
            // �؋��A�C�e���̔w�i
            DrawBox(EVIDENCE_PANEL_X + 10, evidenceY - 2, 
                   EVIDENCE_PANEL_X + EVIDENCE_PANEL_WIDTH - 10, evidenceY + 28,
                   GetColor(70, 60, 50), TRUE);
            DrawBox(EVIDENCE_PANEL_X + 10, evidenceY - 2, 
                   EVIDENCE_PANEL_X + EVIDENCE_PANEL_WIDTH - 10, evidenceY + 28,
                   GetColor(120, 100, 80), FALSE);
            
            // �؋���
            DrawPixelString(EVIDENCE_PANEL_X + 20, evidenceY, colorText, "%d. %s", (int)(i + 1), collectedEvidence[i].c_str());
            
            evidenceY += 35;
            
            // �p�l������͂ݏo���Ȃ��悤��
            if (evidenceY > EVIDENCE_PANEL_Y + EVIDENCE_PANEL_HEIGHT - 40) {
                break;
            }
        }
    }
    
    // ===== �E���F�I�����p�l�� =====
    DrawPixelArtBox(OPTION_PANEL_X, OPTION_PANEL_Y, 
                    OPTION_PANEL_WIDTH, OPTION_PANEL_HEIGHT,
                    colorEvidenceBg, colorBorder);
    
    DrawPixelString(OPTION_PANEL_X + 10, OPTION_PANEL_Y + 10, colorTextSelected, "��⃊�X�g");
    
    // �I������\���i�O���[�A�E�g���ꂽ���̂��܂ށj
    const std::vector<ReasoningOption>& allOptions = reasoningManager->GetAllOptions();
    int selectedIdx = reasoningManager->GetSelectedIndex();
    
    // �I�����̕`��ʒu
    int optionY = OPTION_PANEL_Y + 40;
    const int maxVisibleOptions = 9; // �\���\�ȍő�I������
    const int itemHeight = OPTION_ITEM_HEIGHT;
    const int spacing = OPTION_SPACING;
    
    // �X�N���[���ʒu���v�Z�i�I�𒆂̍��ڂ���Ɍ�����悤�Ɂj
    int scrollOffset = 0;
    if (selectedIdx >= maxVisibleOptions) {
        scrollOffset = selectedIdx - maxVisibleOptions + 1;
    }
    
    // �I������`��
    int visibleCount = 0;
    for (size_t i = 0; i < allOptions.size() && visibleCount < maxVisibleOptions; i++) {
        int displayIndex = (int)i;
        
        // �X�N���[���͈͊O�̓X�L�b�v
        if (displayIndex < scrollOffset) continue;
        if (visibleCount >= maxVisibleOptions) break;
        
        const ReasoningOption& option = allOptions[i];
        bool isSelected = (displayIndex == selectedIdx);

        // エフェクト対応の描画を使用
        ChoiceEffectManager::DrawChoice(
            OPTION_PANEL_X + 10, optionY - 2,
            OPTION_PANEL_WIDTH - 20, itemHeight,
            option,
            isSelected
        );

        optionY += itemHeight + spacing;
        visibleCount++;
    }
    
    // �X�N���[���\�ȏꍇ�̃C���W�P�[�^�[
    if (allOptions.size() > maxVisibleOptions) {
        int scrollBarX = OPTION_PANEL_X + OPTION_PANEL_WIDTH - 15;
        int scrollBarY = OPTION_PANEL_Y + 40;
        int scrollBarHeight = OPTION_PANEL_HEIGHT - 60;
        
        // �X�N���[���o�[�̔w�i
        DrawBox(scrollBarX, scrollBarY, scrollBarX + 5, scrollBarY + scrollBarHeight,
               GetColor(50, 50, 50), TRUE);
        
        // �X�N���[���o�[�̂܂�
        float scrollRatio = (float)scrollOffset / (float)(allOptions.size() - maxVisibleOptions);
        int thumbY = scrollBarY + (int)(scrollRatio * (scrollBarHeight - 20));
        DrawBox(scrollBarX, thumbY, scrollBarX + 5, thumbY + 20,
               GetColor(150, 150, 150), TRUE);
    }
    
    // �������
    int helpY = WINDOW_Y + WINDOW_HEIGHT - 40;
    DrawPixelString(WINDOW_X + 20, helpY, GetColor(180, 180, 180), "����: �I��  Z/����: �m��  X/�L�����Z��: �߂�");
}

