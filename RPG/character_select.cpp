#include "character_select.h"
#include "sprites.h"
#include "raylib.h"
#include "game.h"
#include "main.h"

extern void GoToMainMenuAfterSelect();
CharacterSelectScreen::CharacterSelectScreen()
{
    characters = {
        {"Knight", 1, "擅长近战，高防御。\n拥有强大的生存能力。", 1},
        {"Mage", 2, "擅长远程魔法攻击。\n法力强大，但防御低。", 2},
        {"Archer", 3, "射程远，攻击速度快。\n擅长偷袭与支援。", 3}
    };
}

void CharacterSelectScreen::SetCharacters(const std::vector<int>& spriteList)
{
    for (size_t i = 0; i < spriteList.size() && i < characters.size(); i++)
        characters[i].spriteId = spriteList[i];
}

bool CharacterSelectScreen::IsMouseOverCharacter(int index, Vector2 mouse)
{
    Rectangle area = { 100, float(150 + index * 50), 300, 40 };
    return CheckCollisionPointRec(mouse, area);
}

void CharacterSelectScreen::ConfirmSelection()
{
    //TODO: 设置玩家角色
    //SetPlayerSprite(characters[selectedIndex].spriteId);
    // TODO: 切换到游戏屏幕
    GoToMainMenuAfterSelect();

}

void CharacterSelectScreen::UpdateCharacterSelect()
{
    Vector2 mouse = GetMousePosition();

    // 点击左侧角色列表选择
    for (int i = 0; i < (int)characters.size(); i++)
    {
        if (IsMouseOverCharacter(i, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            selectedIndex = i;
    }

    // 点击确认按钮
    if (CheckCollisionPointRec(mouse, confirmButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        ConfirmSelection();

    // 滚轮切换角色
    float wheel = GetMouseWheelMove();
    if (wheel > 0) selectedIndex = (selectedIndex - 1 + characters.size()) % characters.size();
    if (wheel < 0) selectedIndex = (selectedIndex + 1) % characters.size();

    // 左右按钮点击
    if (CheckCollisionPointRec(mouse, leftBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        selectedIndex = (selectedIndex - 1 + characters.size()) % characters.size();

    if (CheckCollisionPointRec(mouse, rightBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        selectedIndex = (selectedIndex + 1) % characters.size();
}

void CharacterSelectScreen::Draw()
{
    ClearBackground(BLACK);
    DrawText("Select Your Character", 420, 60, 40, WHITE);

    Vector2 mouse = GetMousePosition();

    // 左侧角色列表
    for (int i = 0; i < (int)characters.size(); i++)
    {
        int y = 150 + i * 50;
        Color color = (i == selectedIndex) ? YELLOW : WHITE;

        DrawText(characters[i].name, 100, y, 30, color);
        if (i == selectedIndex)
            DrawRectangleLines(95, y - 5, 260, 40, color);
    }

    // 右侧角色预览
    DrawText("Preview:", 900, 150, 30, WHITE);
    DrawSprite(characters[selectedIndex].spriteId, 950, 300, 0.0f, 4.0f, WHITE, 0);

    // 中间显示角色简介
    DrawCharacterInfo(characters[selectedIndex]);

    // 左右切换按钮
    if (Button(leftBtn, "<"))
    {
        selectedIndex = (selectedIndex - 1 + characters.size()) % characters.size();
    }
    if (Button(rightBtn, ">"))
    {
        selectedIndex = (selectedIndex + 1) % characters.size();
    }

    // 确认按钮
    if (Button(confirmButton, "CONFIRM"))
    {
        ConfirmSelection();
    }
}

void CharacterSelectScreen::DrawCharacterInfo(const CharacterInfo& info)
{
    int screenWidth = GetScreenWidth();
    int fontSize = 25;
    int lineHeight = fontSize + 8;
    int maxWidth = 400;

    std::string desc(info.description);
    size_t pos = 0;
    int startY = 200;

    while(pos < desc.size())
    {
        size_t end = pos;
        std::string line;
        int lineWidth = 0;
        while(end < desc.size())
        {
            std::string nextChar = desc.substr(end,1);
            line += nextChar;
            lineWidth = MeasureText(line.c_str(), fontSize);
            if(nextChar == "\n") { end++; break; }
            if(lineWidth > maxWidth) break;
            end++;
        }
        DrawText(line.c_str(), screenWidth/2 - lineWidth/2, startY, fontSize, GRAY);
        startY += lineHeight;
        pos = end;
    }
}

extern CharacterSelectScreen g_CharacterSelect;
void UpdateCharacterSelect()
{
    g_CharacterSelect.UpdateCharacterSelect();
}


bool CharacterSelectScreen::Button(Rectangle rect, const char* text)
{
    // 检查鼠标是否在按钮上
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool down    = hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // 按钮颜色逻辑与旧版保持一致
    Color color = hovered ? (down ? ButtonPressColor : ButtonHighlight) : ButtonColor;

        // 自动计算字体大小（关键部分）
    // ------------------------------
    int baseFont = ButtonFontSize;          // 你之前设置的默认大小
    int fontSize = baseFont;

    int textWidth  = MeasureText(text, fontSize);
    float textHeight = fontSize;

    float maxWidth  = rect.width  * 0.8f;   // 给点内边距
    float maxHeight = rect.height * 0.7f;

    while ((textWidth > maxWidth || textHeight > maxHeight) && fontSize > 10)
    {
        fontSize--;
        textWidth  = MeasureText(text, fontSize);
        textHeight = fontSize;
    }

    // 绘制按钮背景（带透明效果）
    DrawRectangleRec(rect, ColorAlpha(color, 0.25f));

    // 文本居中绘制
    float textX = rect.x + (rect.width  - textWidth) / 2;
    float textY = rect.y + (rect.height - fontSize) / 2;

    DrawText(text, (int)textX, (int)textY, fontSize, color);

    // 绘制按钮边框
    DrawRectangleLinesEx(rect, 2, color);

    return clicked;
}
