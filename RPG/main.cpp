/**********************************************************************************************
*
*   Raylib RPG Example * A simple RPG made using raylib
*
*    LICENSE: zlib/libpng
*
*   Copyright (c) 2020 Jeffery Myers
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "game.h"
#include "loading.h"
#include "main.h"
#include "map.h"
#include "pause.h"
#include "resource_ids.h"
#include "screens.h"
#include "character_select.h"
#include "audio.h"

#include "raylib.h"
#include <string>



// all the states the program can be in 
enum class ApplicationStates
{
	Startup,
	Loading,
	Menu,
    CharacterSelect, 
	Running,
	Paused,
	GameOver,
	Quitting,
	IntroDialogue,
    VictoryDialogue,
    EndingArt
};
ApplicationStates ApplicationState = ApplicationStates::IntroDialogue;
// 玩家姓名，全局保存
std::string gPlayerName = "???";
Font gChineseFont;

// 对话头像纹理
Texture2D PortraitAunt;
Texture2D PortraitUncle;
Texture2D PortraitStudent;
Texture2D PortraitPlayer;
Texture2D PortraitNarrator;
Texture2D PortraitAuntHappy;   // 胜利对白用的阿姨头像
Texture2D PortraitUncleHappy; 
Texture2D EndingBottom;   // end2.gif
Texture2D EndingTop;      // end1.png

int gVictoryGold = 0;


// the main menu screen
// based on the screen class
class MainMenuScreen : public Screen
{
public:
	void Draw() override
	{
		// dim the background
		DimSceen();

		// title
		DrawCenteredText(40, "Raylib RPG Example", 40, BLUE);

		// version and copyright
		DrawText(VersionString, 2, GetScreenHeight() - 10, 10, GRAY);
		DrawText(CopyrightString, GetScreenWidth() - 2 - MeasureText(CopyrightString, 10), GetScreenHeight() - 10, 10, GRAY);

		// play button
		if (CenteredButton(GetScreenHeight() / 4, "Play"))
			StartGame();
		
		// options button
		CenteredButton(GetScreenHeight()/2, "Options");

		// quit button
		if (CenteredButton(GetScreenHeight() - (GetScreenHeight() / 4), "Quit"))
			QuitApplication();
	}
};
MainMenuScreen MainMenu;

// the game over screen
// shows the win state and final score
class GameOverScreen : public Screen
{
public:
	void Draw() override
	{
		// dim the background
		DimSceen();

		// title
		DrawCenteredText(40, "Raylib RPG Example", 40, BLUE);


		// win state
		if (IsWin)
			DrawCenteredText(120, "Congratulations You WON!", 60, WHITE);
		else
			DrawCenteredText(120, "You died, better luck next time.", 60, RED);

		// score
		DrawCenteredText(200, TextFormat("Score = %d", Gold), 60, YELLOW);

		// version and copyright
		DrawText(VersionString, 2, GetScreenHeight() - 10, 10, GRAY);
		DrawText(CopyrightString, GetScreenWidth() - 2 - MeasureText(CopyrightString, 10), GetScreenHeight() - 10, 10, GRAY);

		// main menu button
		if (CenteredButton(GetScreenHeight() / 2, "Main Menu"))
			GoToMainMenu();

		// quit button
		if (CenteredButton(GetScreenHeight() - (GetScreenHeight() / 4), "Quit"))
			QuitApplication();
	}

	bool IsWin = false;
	int Gold = 0;
};
GameOverScreen GameOver;
// ======================= 开场对话屏幕 =======================

// 简单的对白数组，你可以按自己想法改成南大食堂剧情
// ======================= 开场对话屏幕 =======================

// 说话人类型
enum class Speaker
{
    Narrator,
    Aunt,
    Uncle,
    Student,
    Player,
    AuntHappy,   // 胜利对白用
    UncleHappy
};

struct SimpleLine
{
    Speaker speaker;
    const char* text;
};

static const SimpleLine gIntroLines[] =
{
    { Speaker::Narrator,
      "旁白：最近南京大学的食堂可不太太平，总有小怪跑来捣乱，把食堂阿姨和叔叔忙得团团转，也影响了同学们安心吃饭。"
    },
    { Speaker::Aunt,
      "食堂阿姨（愁眉苦脸）：最近总有小怪来抢饭，给孩子们准备的暖心椰子鸡，结果大家都吃不上……"
    },
    { Speaker::Uncle,
      "后勤叔叔（发愁）：还有怪物在食堂里乱窜，影响同学们进食堂、好好吃饭，老师和同学对后勤部的投诉一桩接一桩啊……"
    },
    { Speaker::Student,
      "同学（小声）：我现在都有点不敢去食堂了，一想到会突然蹦出来小怪，就觉得有点害怕。"
    },
    { Speaker::Uncle,
      "后勤叔叔：你愿意帮帮南大食堂吗？只要能让同学和老师们都吃得开心，我们一定给你一份丰厚的奖励！"
    },
};

struct VictoryLine
{
    Speaker speaker;
    const char* text;
};

static const VictoryLine gVictoryLines[] =
{
    // 后勤叔叔（开心）
    { Speaker::UncleHappy,
      "后勤叔叔（笑眯眯）：同学，没想到你真的打败了心惊豹，守住了我们食堂的镇堂之宝——帝王蟹！"
    },

    // 食堂阿姨（开心）
    { Speaker::AuntHappy,
      "食堂阿姨（开心）：现在再也没有怪物来打扰大家吃饭了，真的是太感谢你啦！"
    },

    // 学生（回忆 + 感谢大家）
    { Speaker::Player,
      "（回想起一路的战斗……每一步都挺不容易的。之所以能坚持下来，是因为——）"
    },
    { Speaker::Player,
      "叔叔阿姨，这不是我一个人的功劳。"
    },
    { Speaker::Player,
      "如果没有阿姨给我的那些热乎饭菜，帮我补足体力……"
    },
    { Speaker::Player,
      "如果没有后勤叔叔一直在背后默默支持，帮我解决了那么多麻烦……"
    },
    { Speaker::Player,
      "如果没有同学们愿意站在我身边，一起抵御怪物的进攻……"
    },
    { Speaker::Player,
      "正是因为大家拧成一股绳，怪物才会落荒而逃，我们才能真正守护好南大食堂。"
    },

    // 同学 A/B/C
    { Speaker::Student,
      "同学A：是啊，叔叔阿姨，虽然我们平时也会吐槽食堂，但有些事，我们一直记在心--每天清晨的一分钱早餐，你们坚持了这么多年。"
    },
    { Speaker::Student,
      "同学B：还有七块钱的学子套餐，荤素搭配、肉菜蛋汤一样不少，让人吃得安心又温暖。"
    },
    { Speaker::Student,
      "同学C：还有一块五的阳春面，香喷喷的，每次路过都忍不住想再来一碗……"
    },

    // 关于帝王蟹 & 媒体
    { Speaker::Narrator,
      "旁白：然而这一次，怪物从头到尾盯上的，其实只有那只帝王蟹——因为在它们眼里，这东西最容易被断章取义、被拿来大做文章。"
    },
    { Speaker::Narrator,
      "它们从来看不见那些用心守住的平价餐，只会盯着那一只标价999元的帝王蟹，大肆渲染、胡乱炒作。"
    },

    { Speaker::Student,
      "同学A：这些我们都看在眼里，可有些所谓的“媒体人”呢？"
    },
    { Speaker::Student,
      "同学B：他们只会截一小块画面、抠一帧画面、放大一个例子，偷换概念、歪曲事实，让人哭笑不得，还摆出一副“公正客观”的样子。"
    },
    { Speaker::Student,
      "同学C：难道南京大学在平价餐饮上已经做到了100分，就不可以继续创新、继续尝试、朝更好的方向多迈一步吗？"
    },

    // 关于“选择”和“象牙塔”的段落
    { Speaker::Narrator,
      "旁白：南京大学从来都不是在“贵”和“便宜”之间二选一，而是想让每个人都有“选择”的权利。"
    },
    { Speaker::Narrator,
      "从学习到生活，从课堂到食堂——南大会优先考虑的是不同需求、不同预算的每一位师生。"
    },
    { Speaker::Narrator,
      "你可以花一分钱吃到暖心的早餐，可以用七块钱吃到营养均衡的套餐，也可以在重要的时刻，偶尔奢侈一回，用一只帝王蟹好好庆祝一场胜利。"
    },
    { Speaker::Narrator,
      "这才是象牙塔应该有的样子——自由、多元，不被偏见和噪音绑架。"
    },

    // 叔叔 & 阿姨最后的话
    { Speaker::UncleHappy,
      "后勤叔叔：孩子，你把我们一直想说的话都说出来了。每一份平价餐、每一次新的尝试，做的都是“怎么让师生吃得更好”，而不是给谁提供炒作的噱头。"
    },
    { Speaker::AuntHappy,
      "食堂阿姨：看到你们吃得香、笑得开心，就是对我们最大的肯定。"
    },
};

static const char* gVictoryFinalNarration =
    "旁白：凶恶的心惊豹已经被赶跑了，热气腾腾的帝王蟹还在轻轻冒着香气。\n"
    "清晨的一分钱早餐、一块五的阳春面、七块钱的学子套餐，依旧在窗口前，安静地等待着新一天的脚步。\n\n"
    "喧嚣终会平息，偏见终会散去，但南京大学食堂的那份温度，会一直留在大家心里——\n"
    "因为这里有踏实的付出，有真实的选择，有一起走过的日常小事，也有你我共同守护的美好生活。";

static const int gVictoryLineCount = sizeof(gVictoryLines) / sizeof(gVictoryLines[0]);

static const int gIntroLineCount = sizeof(gIntroLines) / sizeof(gIntroLines[0]);
static void DrawWrappedText(const char* text, Vector2 pos, float maxWidth, int fontSize, Color color)
{
    Font font = GetFontDefault();
    float spacing = 1.0f;
    const char* lineStart = text;
    float y = pos.y;

    while (*lineStart)
    {
        const char* cursor = lineStart;
        float lineWidth = 0.0f;

        while (*cursor && *cursor != '\n')
        {
            float charWidth = MeasureTextEx(font, std::string(1, *cursor).c_str(), (float)fontSize, spacing).x;
            if (lineWidth + charWidth > maxWidth) break;
            lineWidth += charWidth;
            cursor++;
        }

        std::string line(lineStart, cursor - lineStart);
        DrawTextEx(font, line.c_str(), { pos.x, y }, (float)fontSize, spacing, color);
        y += fontSize + 6;

        if (*cursor == '\n') cursor++;
        lineStart = cursor;
    }
}
extern Font gChineseFont;
static std::string WrapText(const char* text, float maxWidth, int fontSize)
{
    Font font = gChineseFont;
    float spacing = 1.0f;

    std::string wrapped;
    std::string line;

    const char* ptr = text;

    while (*ptr)
    {
        int byteCount = 0;
        // raylib 提供的 UTF-8 解码函数
        int codepoint = GetCodepoint(ptr, &byteCount);
        if (codepoint == 0) break;   // 结束或非法

        // 处理换行符
        if (codepoint == '\n')
        {
            wrapped += line;
            wrapped += '\n';
            line.clear();
            ptr += byteCount;
            continue;
        }

        // 当前这个“字符”的 UTF-8 字节
        std::string ch(ptr, byteCount);

        // 测试：把它加到当前行后是否会超宽
        std::string candidate = line + ch;
        float width = MeasureTextEx(font, candidate.c_str(),
                                    (float)fontSize, spacing).x;

        if (width > maxWidth && !line.empty())
        {
            // 超宽了：先把之前那一行输出，再让 ch 作为新行的第一个字符
            wrapped += line;
            wrapped += '\n';
            line = ch;
        }
        else
        {
            // 没超宽，正常追加
            line = candidate;
        }

        // 移动指针到下一个 codepoint
        ptr += byteCount;
    }

    if (!line.empty())
        wrapped += line;

    return wrapped;
}

void DrawSpeakerLineCommon(Speaker speaker, const char* text, int panelTop, int panelHeight)
{
    Texture2D* portrait = nullptr;
    const char* speakerName = nullptr;

    switch (speaker)
    {
    case Speaker::Aunt:
        portrait = &PortraitAunt;
        speakerName = "食堂阿姨";
        break;
    case Speaker::Uncle:
        portrait = &PortraitUncle;
        speakerName = "后勤叔叔";
        break;
    case Speaker::Student:
        portrait = &PortraitStudent;
        speakerName = "同学";
        break;
    case Speaker::AuntHappy:
        portrait = &PortraitAuntHappy;
        speakerName = "食堂阿姨";
        break;
    case Speaker::UncleHappy:
        portrait = &PortraitUncleHappy;
        speakerName = "后勤叔叔";
        break;
    case Speaker::Player:
        portrait = &PortraitPlayer;
        // 玩家姓名为空时显示“？？？”
        speakerName = gPlayerName.empty() ? "？？？" : gPlayerName.c_str();
        break;
    case Speaker::Narrator:
    default:
        portrait = &PortraitNarrator;
        speakerName = "旁白";
        break;
    }

    int portraitSize = panelHeight - 40;
    int portraitX = 60;
    int portraitY = panelTop + 20;

    if (portrait && portrait->id > 0)
    {
        float scale = (float)portraitSize / (float)portrait->height;
        DrawTextureEx(*portrait, { (float)portraitX, (float)portraitY }, 0.0f, scale, WHITE);
    }
    else
    {
        DrawRectangleLines(portraitX, portraitY, portraitSize, portraitSize, RED);
    }

    float textX = (float)(portraitX + portraitSize + 20);
    float textY = (float)(panelTop + 60);
    float textWidth = (float)(GetScreenWidth() - textX - 40);

    std::string wrapped = WrapText(text, textWidth, 22);
    DrawTextEx(gChineseFont, wrapped.c_str(), { textX, textY }, 22.0f, 1.0f, WHITE);

    // 名字放在头像上方（用中文字体）
    Vector2 namePos = { (float)portraitX, (float)(panelTop + 10) };
    DrawTextEx(gChineseFont, speakerName, namePos, 20.0f, 1.0f, YELLOW);
}
// ================== 对话选项专用按钮（居中、小一号） ==================
static bool DialogueButton(int centerY, const char* text)
{
    int screenW = GetScreenWidth();

    int btnWidth  = 520;   // 按钮宽度，比屏幕窄一点
    int btnHeight = 40;    // 高度比默认小一点，避免挤在一起

    Rectangle rect = {
        (float)(screenW/2 - btnWidth/2),
        (float)(centerY - btnHeight/2),
        (float)btnWidth,
        (float)btnHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    bool pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color bg = hovered ? (Color){ 80, 80, 80, 220 } : (Color){ 40, 40, 40, 200 };

    // 背景 + 边框
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 2, RAYWHITE);

    // 用中文像素字体画文字
    float fontSize = 22.0f;
    float spacing  = 1.0f;
    Vector2 textSize = MeasureTextEx(gChineseFont, text, fontSize, spacing);

    Vector2 textPos = {
        rect.x + (rect.width  - textSize.x)/2.0f,
        rect.y + (rect.height - textSize.y)/2.0f
    };

    DrawTextEx(gChineseFont, text, textPos, fontSize, spacing, WHITE);

    return pressed;
}

// 开场阶段
enum class IntroStage
{
    Lines,      // 前面 5 句
    Choice,     // 三个选项
    AskName,    // 输入名字
    FinalLine   // 最后一行带名字的旁白
};

class IntroDialogueScreen : public Screen
{
public:
    void Reset()
    {
        stage = IntroStage::Lines;
        currentLine = 0;
        finished = false;
        selectedChoice = -1;

        inputBuffer.clear();
        blinkTime = 0.0f;
        showCursor = true;
    }

    void UpdateLogic()
    {
        float dt = GetFrameTime();
        blinkTime += dt;
        if (blinkTime > 0.5f)
        {
            blinkTime = 0.0f;
            showCursor = !showCursor;
        }

        switch (stage)
        {
        case IntroStage::Lines:
            UpdateLines();
            break;
        case IntroStage::Choice:
            // 选项的逻辑直接在 Draw 里通过 CenteredButton 完成，这里不需要处理
            break;
        case IntroStage::AskName:
            HandleNameInput();
            break;
        case IntroStage::FinalLine:
            if (IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_ENTER) ||
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                finished = true;
            }
            break;
        }
    }

    void Draw() override
    {
        DimSceen();

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        int panelHeight = screenH / 3;
        int panelTop = screenH - panelHeight - 40;

        DrawRectangle(40,
                      panelTop,
                      screenW - 80,
                      panelHeight,
                      Fade(BLACK, 0.85f));

        switch (stage)
        {
        case IntroStage::Lines:
            DrawCurrentLine(panelTop, panelHeight);
            break;

        case IntroStage::Choice:
            DrawChoice(panelTop, panelHeight);
            break;

        case IntroStage::AskName:
            DrawAskName(panelTop, panelHeight);
            break;

        case IntroStage::FinalLine:
            DrawFinalLine(panelTop, panelHeight);
            break;
        }
    }

    bool IsFinished() const { return finished; }

private:
    // 阶段一：前 5 句对白
    void UpdateLines()
    {
        if (IsKeyPressed(KEY_SPACE) ||
            IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentLine++;
            if (currentLine >= gIntroLineCount)
            {
                // 进入选项阶段
                stage = IntroStage::Choice;
            }
        }
    }

    void DrawCurrentLine(int panelTop, int panelHeight)
    {
        const SimpleLine& line = gIntroLines[currentLine];
        DrawSpeakerLineCommon(line.speaker, line.text, panelTop, panelHeight);

        const char* hint = "【按 空格 / 回车 / 鼠标左键 继续】";
        Vector2 hintPos = { 60.0f, (float)GetScreenHeight() - 60.0f };
        DrawTextEx(gChineseFont, hint, hintPos, 18.0f, 1.0f, GRAY);
    }


    // 阶段二：三个选项
    // 阶段二：三个选项
    // 阶段二：三个选项
    void DrawChoice(int panelTop, int panelHeight)
    {
        // 改成“玩家”的头像和名字，文本随便写一句心理活动
        const char* playerText =
            " ";

        // 这里用 Player，而不是 Uncle
        DrawSpeakerLineCommon(Speaker::Player, playerText, panelTop, panelHeight);

        // 三个选项的位置保持不变
        int y1 = panelTop + 70;
        int y2 = y1 + 60;
        int y3 = y2 + 60;

        if (DialogueButton(y1, "1. 虽然有点麻烦，不过我来帮帮你们吧。"))
        {
            selectedChoice = 1;
            stage = IntroStage::AskName;
        }
        if (DialogueButton(y2, "2. 好的，我试试看。"))
        {
            selectedChoice = 2;
            stage = IntroStage::AskName;
        }
        if (DialogueButton(y3, "3. 小事一桩，包在我身上！"))
        {
            selectedChoice = 3;
            stage = IntroStage::AskName;
        }
    }



    // 阶段三：输入名字
    void HandleNameInput()
    {
        // 读取所有输入的 codepoint（支持中文）
        int key = GetCharPressed();
        while (key > 0)
        {
            // 简单限制一下总字节数，避免名字太长
            if (inputBuffer.size() < 24)
            {
                int byteSize = 0;
                const char* utf8 = CodepointToUTF8(key, &byteSize);
                if (utf8 != nullptr && byteSize > 0)
                {
                    inputBuffer.append(utf8, byteSize);
                }
            }

            key = GetCharPressed();
        }

        // 退格：按 UTF-8 从尾部删一个 codepoint
        if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer.empty())
        {
            while (!inputBuffer.empty())
            {
                unsigned char c = (unsigned char)inputBuffer.back();
                inputBuffer.pop_back();
                // UTF-8 起始字节：不是 10xxxxxx
                if ((c & 0xC0) != 0x80) break;
            }
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            gPlayerName = inputBuffer.empty() ? "？？？" : inputBuffer;
            stage = IntroStage::FinalLine;
        }
    }


    void DrawAskName(int panelTop, int panelHeight)
    {
        const char* askText = "后勤叔叔：你帮了我们大忙！对了，方便告诉叔叔，你叫什么名字吗？";
        DrawSpeakerLineCommon(Speaker::Uncle, askText, panelTop, panelHeight);

        int boxWidth  = 360;
        int boxHeight = 48;
        int boxX = GetScreenWidth()/2 - boxWidth/2;
        int boxY = panelTop + panelHeight/2 - boxHeight/2;

        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, RAYWHITE);

        // 空的时候显示“请输入名字”
        std::string displayName = inputBuffer.empty() ? "请输入名字" : inputBuffer;
        int textX = boxX + 16;
        int textY = boxY + 12;

        // 用中文字体绘制玩家输入的名字
        Vector2 namePos = { (float)textX, (float)textY };
        DrawTextEx(gChineseFont, displayName.c_str(), namePos, 24.0f, 1.0f, YELLOW);

        // 光标：按中文字体重新测宽
        float textWidth = MeasureTextEx(gChineseFont, displayName.c_str(), 24.0f, 1.0f).x;
        if (showCursor)
        {
            Vector2 cursorPos = { namePos.x + textWidth + 4.0f, (float)textY };
            DrawTextEx(gChineseFont, "|", cursorPos, 24.0f, 1.0f, YELLOW);
        }

        // 中文提示
        const char* hint = "按 回车 键确认姓名";
        Vector2 hintPos = { (float)boxX, (float)(boxY + boxHeight + 30) };
        DrawTextEx(gChineseFont, hint, hintPos, 20.0f, 1.0f, GRAY);
    }


    // 阶段四：最后一行带名字的旁白
    void DrawFinalLine(int panelTop, int panelHeight)
    {
        char buffer[512];
        const char* nameForText = gPlayerName.empty() ? "？？？" : gPlayerName.c_str();
        snprintf(buffer, sizeof(buffer),
            "旁白：%s，我们终于等到你啦！现在，让我们一起出发，守护南京大学的食堂吧！",
            nameForText);

        DrawSpeakerLineCommon(Speaker::Narrator, buffer, panelTop, panelHeight);

        const char* hint = "【按 空格 / 回车 / 鼠标左键 继续】";
        Vector2 hintPos = { 60.0f, (float)GetScreenHeight() - 60.0f };
        DrawTextEx(gChineseFont, hint, hintPos, 18.0f, 1.0f, GRAY);
    }

    // 通用：画头像 + 姓名 + 文本
    

private:
    IntroStage  stage        = IntroStage::Lines;
    int         currentLine  = 0;
    bool        finished     = false;
    int         selectedChoice = -1;

    std::string inputBuffer;
    float       blinkTime    = 0.0f;
    bool        showCursor   = true;
};
enum class VictoryStage
{
    Lines,          // 一句句对白
    FinalNarration  // 最后一整屏旁白
};

class VictoryDialogueScreen : public Screen
{
public:
    void Reset()
    {
        stage = VictoryStage::Lines;
        currentIndex = 0;
        finished = false;
    }

    void UpdateLogic()
    {
        switch (stage)
        {
        case VictoryStage::Lines:
            if (IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_ENTER) ||
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentIndex++;
                if (currentIndex >= gVictoryLineCount)
                {
                    stage = VictoryStage::FinalNarration;
                }
            }
            break;

        case VictoryStage::FinalNarration:
            if (IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_ENTER) ||
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                finished = true;
            }
            break;
        }
    }

    void Draw() override
    {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        if (stage == VictoryStage::Lines)
        {
            DimSceen();

            int panelHeight = screenH / 3;
            int panelTop    = screenH - panelHeight - 40;

            DrawRectangle(40,
                          panelTop,
                          screenW - 80,
                          panelHeight,
                          Fade(BLACK, 0.85f));

            const VictoryLine& line = gVictoryLines[currentIndex];
            DrawSpeakerLineCommon(line.speaker, line.text, panelTop, panelHeight);

            const char* hint = "【按 空格 / 回车 / 鼠标左键 继续】";
            Vector2 hintPos = { 60.0f, (float)screenH - 60.0f };
            DrawTextEx(gChineseFont, hint, hintPos, 18.0f, 1.0f, GRAY);

        }
        else // FinalNarration
        {
            // 整屏黑底 + 居中文字
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.9f));

            int margin = 80;
            int fontSize = 24;
            float maxWidth = (float)(screenW - margin * 2);

            std::string wrapped = WrapText(gVictoryFinalNarration, maxWidth, fontSize);
            DrawTextEx(gChineseFont,
                    wrapped.c_str(),
                    { (float)margin, (float)margin },
                    (float)fontSize,
                    1.0f,
                    WHITE);

            const char* hint = "【按 空格 / 回车 / 鼠标左键 继续】";
            Vector2 hintPos = { (float)margin, (float)screenH - 60.0f };
            DrawTextEx(gChineseFont, hint, hintPos, 18.0f, 1.0f, GRAY);
        }
    }

    bool IsFinished() const { return finished; }

private:
    VictoryStage stage      = VictoryStage::Lines;
    int          currentIndex = 0;
    bool         finished   = false;
};
class EndingScreen : public Screen
{
public:
    void Draw() override
    {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // 黑色背景
        DrawRectangle(0, 0, screenW, screenH, BLACK);

        // 先画底图：end2.gif
        if (EndingBottom.id != 0)
        {
            Rectangle src = { 0, 0, (float)EndingBottom.width, (float)EndingBottom.height };
            Rectangle dst = { 0, 0, (float)screenW, (float)screenH };
            DrawTexturePro(EndingBottom, src, dst, { 0, 0 }, 0.0f, WHITE);
        }

        // 再画上层：end1.png
        if (EndingTop.id != 0)
        {
            Rectangle src = { 0, 0, (float)EndingTop.width, (float)EndingTop.height };
            Rectangle dst = { 0, 0, (float)screenW, (float)screenH };
            DrawTexturePro(EndingTop, src, dst, { 0, 0 }, 0.0f, WHITE);
        }

        // 底部提示
        const char* hint = "按任意键继续";
        Vector2 size = MeasureTextEx(gChineseFont, hint, 24.0f, 1.0f);
        Vector2 pos  = { (float)screenW / 2 - size.x / 2.0f,
                         (float)screenH - 80.0f };
        DrawTextEx(gChineseFont, hint, pos, 24.0f, 1.0f, RAYWHITE);
    }
};

// ★ 全局实例
IntroDialogueScreen IntroScreen;
VictoryDialogueScreen VictoryScreen;
CharacterSelectScreen CharacterSelect;
CharacterSelectScreen g_CharacterSelect;
EndingScreen EndingArtScreen;   // 新增


// 开场对话开始：重置状态并设为当前屏幕
void StartIntroDialogue()
{
    IntroScreen.Reset();
    SetActiveScreen(&IntroScreen);
}

// 每帧更新开场对话，结束后切到主菜单
void UpdateIntroDialogue()
{
    IntroScreen.UpdateLogic();

    if (IntroScreen.IsFinished())
    {
        ApplicationState = ApplicationStates::CharacterSelect;
        SetActiveScreen(&CharacterSelect);
    }
}
void UpdateVictoryDialogue()
{
    VictoryScreen.UpdateLogic();

    if (VictoryScreen.IsFinished())
    {
        // ★ 胜利对白结束，进入结尾叠图界面
        ApplicationState = ApplicationStates::EndingArt;
        SetActiveScreen(&EndingArtScreen);
        // GameOver 的 win / gold 在 UpdateEndingArt 里再设置
    }
}

void UpdateEndingArt()
{
    // 按任意键 / 鼠标左键 结束，进入结算界面
    if (IsKeyPressed(KEY_SPACE) ||
        IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_ESCAPE) ||
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        ApplicationState = ApplicationStates::GameOver;
        SetActiveScreen(&GameOver);
        GameOver.IsWin = true;
        GameOver.Gold = gVictoryGold;
    }
}

void UpdateCharacterSelectState()
{
    // 如果你的类里有别的更新函数名，就改成对应的名字
    CharacterSelect.UpdateCharacterSelect();
}

// ============================================================

// setup the window and icon
void SetupWindow()
{
	// Validate that the window is not taller than the monitor size, if so, set it to a smaller size
	int monitor = GetCurrentMonitor();

	int maxHeight = GetMonitorHeight(monitor) - 40;
	if (GetScreenHeight() > maxHeight)
		SetWindowSize(GetScreenWidth(), maxHeight);

	SetExitKey(0);
	SetTargetFPS(144);

	// load an image for the window icon
	Image icon = LoadImage("icons/Icon.6_98.png");

	// ensure that the picture is in the correct format
	ImageFormat(&icon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	// replace the background and border colors with transparent
	ImageColorReplace(&icon, BLACK, BLANK);
	ImageColorReplace(&icon, Color{ 136,136,136,255 }, BLANK);

	// set the icon
	SetWindowIcon(icon);

	// free the image data
	UnloadImage(icon);
}

// called by the loading system when all assets are loaded
// called by the loading system when all assets are loaded
void LoadComplete()
{
    // 資源加載完成後，先進入開場對話狀態
    ApplicationState = ApplicationStates::IntroDialogue;

    // 啟動對話屏幕
    StartIntroDialogue();

    // 順便把菜單背景地圖提前加載好（開場對話時可作為背景）
    LoadMap("maps/menu_map.tmx");
}


// called when the game wants to go back to the main menu, from pause or game over screens
void GoToMainMenu()
{
	// quit our game, if our game was running
	if (ApplicationState == ApplicationStates::Running || ApplicationState == ApplicationStates::Paused)
		QuitGame();

	// start our background music again
	StartBGM("sounds/Flowing Rocks.ogg");

	// go back to the main menu like we did when we started up
	LoadComplete();
}
// 角色选择界面选完角色之后，回到主菜单
void GoToMainMenuAfterSelect()
{
    ApplicationState = ApplicationStates::Menu;
    SetActiveScreen(&MainMenu);

    // 这里看你要不要放 BGM
    // StartBGM("sounds/Flowing Rocks.ogg");
}

// called by the main menu to check for exit
void UpdateMainMenu()
{
	if (IsKeyPressed(KEY_ESCAPE))
		QuitApplication();
}

// starts a new game
void StartGame()
{
	ApplicationState = ApplicationStates::Running;
	SetActiveScreen(nullptr);
	StopBGM();
	InitGame();
}

// called when the menu wants to pause the game
void PauseGame()
{
	ApplicationState = ApplicationStates::Paused;
}

// called when the menu wants to resume the game
void ResumeGame()
{
	ApplicationState = ApplicationStates::Running;
	SetActiveScreen(nullptr);
	ActivateGame();
}

// called by the game when it is over, by win or loss
void EndGame(bool win, int gold)
{
    if (win)
    {
        // 记录金币，先播放胜利对白
        gVictoryGold = gold;
        VictoryScreen.Reset();
        SetActiveScreen(&VictoryScreen);
        ApplicationState = ApplicationStates::VictoryDialogue;
    }
    else
    {
        // 失败仍然走原来的 GameOver 流程
        ApplicationState = ApplicationStates::GameOver;
        SetActiveScreen(&GameOver);
        GameOver.IsWin = false;
        GameOver.Gold = gold;
    }
}


// quit the entire application
void QuitApplication()
{
	ApplicationState = ApplicationStates::Quitting;
}

bool SearchAndSetResourceDir(const char* folderName)
{
	// check the working dir
	if (DirectoryExists(folderName))
	{
		ChangeDirectory(TextFormat("%s/%s", GetWorkingDirectory(), folderName));
		return true;
	}

	const char* appDir = GetApplicationDirectory();

	// check the applicationDir
	const char* dir = TextFormat("%s%s", appDir, folderName);
	if (DirectoryExists(dir))
	{
		ChangeDirectory(dir);
		return true;
	}

	// check one up from the app dir
	dir = TextFormat("%s../%s", appDir, folderName);
	if (DirectoryExists(dir))
	{
		ChangeDirectory(dir);
		return true;
	}

	// check two up from the app dir
	dir = TextFormat("%s../../%s", appDir, folderName);
	if (DirectoryExists(dir))
	{
		ChangeDirectory(dir);
		return true;
	}

	// check three up from the app dir
	dir = TextFormat("%s../../../%s", appDir, folderName);
	if (DirectoryExists(dir))
	{
		ChangeDirectory(dir);
		return true;
	}

	return false;
}

// the main application loop
int main()
{
	// setup the window
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280,700,"RPG Example");
	SetupWindow();

	SearchAndSetResourceDir("_resources");
    InitAudio();
    InitResources();

    //----------------- 加载中文字体：ASCII + 常用汉字 -----------------
        //----------------- 加载中文字体：ASCII + 汉字 + 标点 -----------------
    // 1) 英文/数字/常用符号：32 ~ 126
    const int basicStart = 32;
    const int basicEnd   = 126;

    // 2) 常用汉字：CJK Unified Ideographs
    const int cjkStart   = 0x4E00;   // 一
    const int cjkEnd     = 0x9FA5;   // 常用汉字大致范围

    // 3) 常用中文标点的几个区段：
    //    - CJK 符号与标点：   0x3000 ~ 0x303F（、 。 「 」 等）
    //    - 通用标点：         0x2000 ~ 0x206F（… 等）
    //    - 全角形式：         0xFF00 ~ 0xFFEF（， 。 ！ ？ ： 等全角符号）

    const int punct1Start = 0x3000;
    const int punct1End   = 0x303F;

    const int punct2Start = 0x2000;
    const int punct2End   = 0x206F;

    const int punct3Start = 0xFF00;
    const int punct3End   = 0xFFEF;

    int basicCount  = basicEnd  - basicStart  + 1;
    int cjkCount    = cjkEnd    - cjkStart    + 1;
    int punct1Count = punct1End - punct1Start + 1;
    int punct2Count = punct2End - punct2Start + 1;
    int punct3Count = punct3End - punct3Start + 1;

    int totalCount = basicCount + cjkCount
                   + punct1Count + punct2Count + punct3Count;

    int* codepoints = (int*)MemAlloc(totalCount * sizeof(int));
    int index = 0;

    // 英文/数字/基础符号
    for (int cp = basicStart; cp <= basicEnd;   ++cp) codepoints[index++] = cp;
    // 常用汉字
    for (int cp = cjkStart;   cp <= cjkEnd;     ++cp) codepoints[index++] = cp;
    // CJK 标点
    for (int cp = punct1Start; cp <= punct1End; ++cp) codepoints[index++] = cp;
    // 通用标点（含 … 等）
    for (int cp = punct2Start; cp <= punct2End; ++cp) codepoints[index++] = cp;
    // 全角符号（， 。 ： ！ ？ “ ” 等）
    for (int cp = punct3Start; cp <= punct3End; ++cp) codepoints[index++] = cp;

    gChineseFont = LoadFontEx("fonts/Zpix.ttf", 24, codepoints, totalCount);
    MemFree(codepoints);

    if (gChineseFont.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "Failed to load Chinese font!");
    }
    //----------------- 中文字体加载结束 -----------------

// 加载头像纹理（路径按你放的为准）
	auto LoadPortrait = [](Texture2D& tex, const char* path)
    {
        tex = LoadTexture(path);
        if (tex.id == 0)
        {
            TraceLog(LOG_WARNING, TextFormat("Portrait texture missing: %s", path));
        }
    };

    LoadPortrait(PortraitAunt,     "portraits/aunt.png");
    LoadPortrait(PortraitUncle,    "portraits/uncle.png");
    LoadPortrait(PortraitStudent,  "portraits/student.png");
    LoadPortrait(PortraitPlayer,   "portraits/player.png");
    LoadPortrait(PortraitNarrator, "portraits/narrator.png");
    LoadPortrait(PortraitAuntHappy,   "portraits/aunt2.png");
    LoadPortrait(PortraitUncleHappy,  "portraits/uncle2.png");
     LoadPortrait(PortraitAuntHappy,   "portraits/aunt2.png");
    LoadPortrait(PortraitUncleHappy,  "portraits/uncle2.png");

    // ★ 结尾叠图贴图（文件直接放在 _resources 目录下）
    EndingBottom = LoadTexture("end2.gif");
    EndingTop    = LoadTexture("end1.png");
    if (EndingBottom.id == 0) TraceLog(LOG_WARNING, "Failed to load end2.gif");
    if (EndingTop.id == 0)    TraceLog(LOG_WARNING, "Failed to load end1.png");

    
	ApplicationState = ApplicationStates::Loading;

	// game loop
	while (!WindowShouldClose() && ApplicationState != ApplicationStates::Quitting)
	{
		// call the update that goes with our current game state
		switch (ApplicationState)
		{
		case ApplicationStates::IntroDialogue:
            UpdateIntroDialogue();
            break;

        case ApplicationStates::VictoryDialogue:          // 新增
            UpdateVictoryDialogue();
            break;

        case ApplicationStates::CharacterSelect:   // ← 新增
            UpdateCharacterSelectState();
            break;

        case ApplicationStates::EndingArt:        // ★ 新增
            UpdateEndingArt();
            break;

		case ApplicationStates::Loading:
			UpdateLoad();
			break;

		case ApplicationStates::Menu:
			UpdateMainMenu();
			break;

		case ApplicationStates::Running:
			UpdateGame();
			break;

		case ApplicationStates::Paused: 
			UpdatePaused();
			break;
		}

		// update the screen for this frame
		BeginDrawing();
		ClearBackground(BLACK);

		// the map is always first because it is always under the menu
		DrawMap();

		// draw whatever menu or hud screen we have
		DrawScreen();

		UpdateAudio();
		EndDrawing();
	}

	ShutdownAudio();
	CleanupResources();


// 卸载头像
	UnloadTexture(PortraitAunt);
	UnloadTexture(PortraitUncle);
	UnloadTexture(PortraitStudent);
	UnloadTexture(PortraitPlayer);
	UnloadTexture(PortraitNarrator);
    UnloadTexture(PortraitAuntHappy);
    UnloadTexture(PortraitUncleHappy);
    UnloadTexture(PortraitNarrator);
    UnloadTexture(PortraitAuntHappy);
    UnloadTexture(PortraitUncleHappy);

    // ★ 卸载结尾贴图
    UnloadTexture(EndingBottom);
    UnloadTexture(EndingTop);

    UnloadFont(gChineseFont);

	CloseWindow();

	return 0;
}
