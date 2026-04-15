#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include "screens.h"

class CharacterSelectScreen : public Screen
{
public:
    CharacterSelectScreen();

    void SetCharacters(const std::vector<int>& spriteList);
    void UpdateCharacterSelect();  // 保留原来的更新函数
    void Draw();
    void ConfirmSelection();
    bool Button(Rectangle rect, const char* text);

private:
    struct CharacterInfo {
        const char* name;
        int id;
        const char* description;
        int spriteId;
    };

    std::vector<CharacterInfo> characters;
    int selectedIndex = 0;

    Rectangle confirmButton = { 500, 500, 300, 60 };
    Rectangle leftBtn = { 420, 400, 80, 60 };
    Rectangle rightBtn = { 800, 400, 80, 60 };

    bool IsMouseOverCharacter(int index, Vector2 mouse);
    void DrawCharacterInfo(const CharacterInfo& info);
};

void UpdateCharacterSelect();