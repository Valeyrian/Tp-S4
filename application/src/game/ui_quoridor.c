/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "game/ui_quoridor.h"
#include "game/game_common.h"
#include "game/scene.h"
#include "core/graph.h"
#include "core/quoridor_ai.h"
#include <time.h>
#include <core/listData.h>

static bool UIQuoridor_isPlayerTurn(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");
    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);
    const int selectedMode = UIList_getSelected(self->m_listMode);
    return (selectedMode == 0) || ((selectedMode == 1) && core->playerID == 0);
}

void UIQuoridor_updateTurn(UIQuoridor *self)
{

    assert(self && "The UIQuoridor must be created");

    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);
    if (core->state != QUORIDOR_STATE_IN_PROGRESS) return;

    bool playerTurn = UIQuoridor_isPlayerTurn(self);
    clock_t endTime = clock();

    core->timeElapsed = (float)(endTime - core->startTime) / CLOCKS_PER_SEC; 
  

    if (playerTurn == false)
    {
        // Tour de l'IA
        if (self->m_aiTurn.action == QUORIDOR_ACTION_UNDEFINED)
        {
            self->m_aiAccu = 0;
            int depth = 1;
            switch (UIList_getSelected(self->m_listLevel))
            {
            default:
            case 0: depth = 2; break;
            case 1: depth = 3; break;
            case 2: depth = 5; break;
            }



           
            int currentPlayerBeforeMove = core->playerID;
            
            self->m_aiTurn = QuoridorCore_computeTurn(core, depth, self->m_aiData[core->playerID]);
           
            
            QuoridorData data;
            data.action = self->m_aiTurn.action;
            data.destPos.i = self->m_aiTurn.i;
            data.destPos.j = self->m_aiTurn.j;
            data.originPos = core->positions[currentPlayerBeforeMove]; 


           
            AIData_add(self->m_aiData[currentPlayerBeforeMove], data);


            clock_t startTime = clock();

        }
        else
        {

            core->startTime = clock();

            self->m_aiAccu += Timer_getDeltaMS(g_time);

            Uint64 minTime = 0;
            switch (UIList_getSelected(self->m_listCPUTime))
            {
            default:
            case 0: minTime = 0; break;
            case 1: minTime = 100; break;
            case 2: minTime = 500; break;
            case 3: minTime = 1000; break;
            }

            if (self->m_aiAccu > minTime)
            {
                QuoridorCore_playTurn(core, self->m_aiTurn);
                self->m_aiTurn.action = QUORIDOR_ACTION_UNDEFINED;
                return;
            }
        }
    }
    else 
    {
        // Tour du joueur

        clock_t endTime;


        const int gridSize = core->gridSize;
        Input *input = Scene_getInput(self->m_scene);
        if (input->validatePressed == false) return;

        
        int currentPlayerBeforeMove = core->playerID;

        Vec2 mousePos = input->mousePos;
        for (int i = 0; i < gridSize; i++)
        {
            for (int j = 0; j < gridSize; j++)
            {
                if (FRect_containsPoint(&(self->m_rectCells[i][j]), mousePos))
                {
                    if (QuoridorCore_canMoveTo(core, i, j))
                    {

                        
                        QuoridorData data;
                        data.action = QUORIDOR_MOVE_TO;
                        data.destPos.i = i;
                        data.destPos.j = j;
                        data.originPos = core->positions[currentPlayerBeforeMove]; // Position avt mvt 

                        

                        QuoridorCore_moveTo(core, i, j);


                        core->startTime = clock();
                      
                        AIData_add(self->m_aiData[currentPlayerBeforeMove], data);


                        return; 
                    }
                }
            }
        }

        for (int i = 0; i < gridSize - 1; i++)
        {
            for (int j = 0; j < gridSize - 1; j++)
            {
                if (FRect_containsPoint(&(self->m_rectMouseHWalls[i][j]), mousePos))
                {
                    if (QuoridorCore_canPlayWall(core, WALL_TYPE_HORIZONTAL, i, j))
                    {
                        endTime = clock();

                        // CORRECTION 7: Enregistrer AVANT de poser le mur
                        QuoridorData data;
                        data.action = QUORIDOR_PLAY_HORIZONTAL_WALL;
                        data.destPos.i = i;
                        data.destPos.j = j;
                        data.originPos = core->positions[currentPlayerBeforeMove];

                        QuoridorCore_playWall(core, WALL_TYPE_HORIZONTAL, i, j);

                        core->startTime = clock();

                        //QuoridorCore_print(core);


                        // CORRECTION 8: Enregistrer dans la liste du BON joueur
                        AIData_add(self->m_aiData[currentPlayerBeforeMove], data);
                        return; // Important: sortir après avoir traité le mur

                    }
                }
                if (FRect_containsPoint(&(self->m_rectMouseVWalls[i][j]), mousePos))
                {
                    if (QuoridorCore_canPlayWall(core, WALL_TYPE_VERTICAL, i, j))
                    {

                        core->startTime = clock();

                        // CORRECTION 9: Enregistrer AVANT de poser le mur
                        QuoridorData data;
                        data.action = QUORIDOR_PLAY_VERTICAL_WALL;
                        data.destPos.i = i;
                        data.destPos.j = j;
                        data.originPos = core->positions[currentPlayerBeforeMove];

                        QuoridorCore_playWall(core, WALL_TYPE_VERTICAL, i, j);

                        // CORRECTION 10: Enregistrer dans la liste du BON joueur
                        AIData_add(self->m_aiData[currentPlayerBeforeMove], data);
                        return; // Important: sortir après avoir traité le mur

                    }
                }
            }
        }
    }
}

static float UIQuoridor_renderPanel(
    UIQuoridor *self, float x, float y, float w,
    Text *title, Text **columns, int columnCount)
{
    assert(self && "The UIQuoridor must be created");
    float titleH = (float)Text_getHeight(title);
    float titleW = (float)Text_getWidth(title);
    float textH = (float)Text_getHeight(columns[0]);
    float textW = 0.f;
    const float vPadding = 20.f;
    const float hPadding = 50.f;
    const float panelHeight = titleH + textH + 2 * vPadding;
    const float titleSep = 2.f;

    SDL_FRect dstRect = { 0 };
    dstRect.x = roundf(x);
    dstRect.y = roundf(y);
    dstRect.w = roundf(w);
    dstRect.h = roundf(panelHeight);
    Game_setRenderDrawColor(g_colors.cell, 255);
    SDL_RenderFillRect(g_renderer, &dstRect);

    dstRect.h = 10.f;
    Game_setRenderDrawColor(g_colors.selected, 255);
    SDL_RenderFillRect(g_renderer, &dstRect);

    y += vPadding;

    SDL_Texture *texture = Text_getTexture(title);
    dstRect.x = roundf(x + hPadding);
    dstRect.y = roundf(y);
    dstRect.w = titleW;
    dstRect.h = titleH;
    SDL_RenderTexture(g_renderer, texture, NULL, &dstRect);

    y += dstRect.h + titleSep;

    for (int i = 0; i < columnCount; i++)
    {
        texture = Text_getTexture(columns[i]);
        Text_getSize(columns[i], &textW, &textH);
        dstRect.x = roundf(x + hPadding + i * (1.f/ columnCount) * (w - 2 * hPadding));



        dstRect.y = roundf(y);
        dstRect.w = textW;
        dstRect.h = textH;
        SDL_RenderTexture(g_renderer, texture, NULL, &dstRect);
    }

    y += vPadding + textH;

    return y;
}

UIQuoridor *UIQuoridor_create(Scene *scene)
{
    UIQuoridor *self = (UIQuoridor *)calloc(1, sizeof(UIQuoridor));
    AssertNew(self);

    self->m_scene = scene;
    self->m_aiTurn.action = QUORIDOR_ACTION_UNDEFINED;

    QuoridorCore *core = Scene_getQuoridorCore(scene);

    core->playerCount = 2;
    for (int i = 0; i < 8; i++)
    {
        self->m_aiData[i] = AIData_create(core); 
    }

    AssetManager *assets = Scene_getAssetManager(scene);
    TTF_Font *font = NULL;
    SDL_FRect rect = { .x = 0.f, .y = 0.f, .w = 100.f, .h = 100.f };

    // Listes pour les settings
    font = AssetManager_getFont(assets, FONT_NORMAL);
    const char *modeValues[] = { "P vs P", "P vs CPU", "CPU vs CPU" };
    self->m_listMode = UIList_create(
        scene, rect, 0.5f, font,
        "Mode", modeValues, sizeof(modeValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listMode, 0);

    const char *levelValues[] = { "easy", "medium", "hard" };
    self->m_listLevel = UIList_create(
        scene, rect, 0.5f, font,
        "CPU Level", levelValues, sizeof(levelValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listLevel, 0);

    const char *timeValues[] = { "disabled", "100 ms", "500 ms", "1 s" };
    self->m_listCPUTime = UIList_create(
        scene, rect, 0.5f, font,
        "CPU minimal time", timeValues, sizeof(timeValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listCPUTime, 2);

    const char *gridValues[] = { "5 x 5", "7 x 7", "9 x 9" };
    self->m_listGridSize = UIList_create(
        scene, rect, 0.5f, font,
        "Grid size", gridValues, sizeof(gridValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listGridSize, 2);

    const char *wallValues[] = { "5 walls", "6 walls", "7 walls", "8 walls", "9 walls", "10 walls" };
    self->m_listWallCount = UIList_create(
        scene, rect, 0.5f, font,
        "Mode", wallValues, sizeof(wallValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listWallCount, 5);

    const char *randStartValues[] = { "No", "Yes" };
    self->m_listRandStart = UIList_create(
        scene, rect, 0.5f, font,
        "Random start", randStartValues, sizeof(randStartValues) / sizeof(char *),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listRandStart, 0);


    const char* playerValues[] = { "2", "4", "8"};
    self->m_listPlayerCount = UIList_create(
        scene, rect, 0.5f, font,
        "PLayers count", playerValues, sizeof(playerValues) / sizeof(char*),
        g_colors.white, g_colors.cell, g_colors.selected
    );
    UIList_setSelected(self->m_listPlayerCount, 0);
    
    // Boutons
    self->m_buttonSettings = UIButton_create(
        scene, rect, font, "Settings",
        g_colors.white, g_colors.cell, g_colors.selected
    );
    self->m_buttonRestart = UIButton_create(
        scene, rect, font, "Restart",
        g_colors.white, g_colors.cell, g_colors.selected
    );
    self->m_buttonBack = UIButton_create(
        scene, rect, font, "Back",
        g_colors.white, g_colors.cell, g_colors.selected
    );
    self->m_buttonActionBack = UIButton_create( 
        scene, rect, font, "<-",
        g_colors.white, g_colors.cell, g_colors.selected 
    );
    self->m_buttonCheatSwitch = UIButton_create(
        scene, rect, font, "Preview",
        g_colors.white, g_colors.cell, g_colors.selected 
    );
    // Textes
    font = AssetManager_getFont(assets, FONT_NORMAL);
    self->m_textTitleInfo = Text_create(g_renderer, font, "Information", g_colors.white);
    self->m_textTitleWalls = Text_create(g_renderer, font, "Walls", g_colors.white);
    self->m_textTitleDistances = Text_create(g_renderer, font, "Distances", g_colors.white);
    self->m_textTime[0] = Text_create(g_renderer, font, "0.00", g_colors.white);
    self->m_textTitleTime = Text_create(g_renderer, font, "Time", g_colors.white);

    font = AssetManager_getFont(assets, FONT_BIG);
    self->m_textTitleSettings = Text_create(g_renderer, font, "Settings", g_colors.player[0]);
    self->m_textInfo = Text_create(g_renderer, font, "P1 turn", g_colors.player[0]);
    self->m_textWalls[0] = Text_create(g_renderer, font, "10", g_colors.player[0]);

    for (int c = 0; c < 8; c++)
    {
        self->m_textWalls[c] = Text_create(g_renderer, font, "10", g_colors.player[c]);
        self->m_textDistances[c] = Text_create(g_renderer, font, "10", g_colors.player[c]);
    }




    // Calcule les positions des rectangles/boutons/listes
    UIQuoridor_updateRects(self);

    return self;
}

void UIQuoridor_destroy(UIQuoridor *self)
{
    if (!self) return;

    for (int i = 0; i < 8; i++)
    {
        AIData_destroy(self->m_aiData[i]);
        Text_destroy(self->m_textWalls[i]);
        Text_destroy(self->m_textDistances[i]);
    }

    Text_destroy(self->m_textTitleInfo);
    Text_destroy(self->m_textTitleWalls);
    Text_destroy(self->m_textTitleDistances);
    Text_destroy(self->m_textTitleSettings);


    Text_destroy(self->m_textInfo);

    UIList_destroy(self->m_listMode);
    UIList_destroy(self->m_listLevel);
    UIList_destroy(self->m_listCPUTime);
    UIList_destroy(self->m_listGridSize);
    UIList_destroy(self->m_listWallCount);
    UIList_destroy(self->m_listRandStart);

    UIButton_destroy(self->m_buttonSettings);
    UIButton_destroy(self->m_buttonRestart);
    UIButton_destroy(self->m_buttonBack);
	UIButton_destroy(self->m_buttonActionBack);
	UIButton_destroy(self->m_buttonCheatSwitch);

    free(self);
}

void UIQuoridor_update(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    UIQuoridor_updateRects(self);

    if (self->m_inSettings)
    {
        UIQuoridor_updatePageSettings(self);
    }
    else
    {
        UIQuoridor_updatePageMain(self);
    }
}

void UIQuoridor_updatePageMain(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);

    UIButton_update(self->m_buttonSettings);
    UIButton_update(self->m_buttonRestart);
    UIButton_update(self->m_buttonActionBack);
	UIButton_update(self->m_buttonCheatSwitch);


  char buffer2[128] = { 0 };
    sprintf(buffer2, "%.2f", core->timeElapsed);

    Text_setString(self->m_textTime[0], buffer2);
    for(int c = 0;c<core->playerCount;c++)
        if (core->playerID == c)
            Text_setColor(self->m_textTime[0], g_colors.player[c]);



    if (UIButton_isPressed(self->m_buttonSettings))
    {
        self->m_inSettings = true;
    }
    else if (UIButton_isPressed(self->m_buttonRestart))
    {
        UIQuoridor_restartQuoridor(self);
    }
    else if (UIButton_isPressed(self->m_buttonActionBack))
    {
        QuoridorCore_Undo(core, self);
    }
    else if (UIButton_isPressed(self->m_buttonCheatSwitch))
    {
		self->m_isCheatEnable = !self->m_isCheatEnable; 
    }
    else
    {
        UIQuoridor_updateTurn(self);
    }

    char buffer[128] = { 0 };

    for (int i = 0; i < core->playerCount; i++)
    {
        sprintf(buffer, "%d", core->wallCounts[i]);
        Text_setString(self->m_textWalls[i], buffer);

        QuoridorPos path[MAX_GRID_SIZE * MAX_GRID_SIZE];
        int size = 0;
#ifndef A_STAR
        size = BFS_search2(core, i, path);
#else
		size = AStar_search(core, i, path);
#endif

     

        sprintf(buffer, "%d", size - 1);
        Text_setString(self->m_textDistances[i], buffer);
    }

    const bool playerTurn = UIQuoridor_isPlayerTurn(self);
    if (core->state == QUORIDOR_STATE_IN_PROGRESS)
    {
        if (playerTurn)
            sprintf(buffer, "Player %d turn", core->playerID + 1);
        else
            sprintf(buffer, "CPU %d turn", core->playerID + 1);

        Text_setString(self->m_textInfo, buffer);
        for (int c = 0; c < core->playerCount; c++)
            if (core->playerID == c)
                Text_setColor(self->m_textInfo, g_colors.player[c]);

    }
    else
    {
        if (core->state == QUORIDOR_STATE_P0_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 1 WON" : "CPU 1 WON");
            Text_setColor(self->m_textInfo, g_colors.player[0]);
        }
        else if (core->state == QUORIDOR_STATE_P1_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 2 WON" : "CPU 2 WON");
            Text_setColor(self->m_textInfo, g_colors.player[1]);
        }

        else if (core->state == QUORIDOR_STATE_P2_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 3 WON" : "CPU 3 WON");
            Text_setColor(self->m_textInfo, g_colors.player[2]);
        }
        else if (core->state == QUORIDOR_STATE_P3_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 4 WON" : "CPU 4 WON");
            Text_setColor(self->m_textInfo, g_colors.player[3]);
        }
        else if (core->state == QUORIDOR_STATE_P4_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 5 WON" : "CPU 5 WON");
            Text_setColor(self->m_textInfo, g_colors.player[3]);
        }
        else if (core->state == QUORIDOR_STATE_P5_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 6 WON" : "CPU 6 WON");
            Text_setColor(self->m_textInfo, g_colors.player[3]);
        }
        else if (core->state == QUORIDOR_STATE_P6_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 7 WON" : "CPU 7 WON");
            Text_setColor(self->m_textInfo, g_colors.player[3]);
        }
        else if (core->state == QUORIDOR_STATE_P7_WON)
        {
            Text_setString(self->m_textInfo, playerTurn ? "Player 8 WON" : "CPU 8 WON");
            Text_setColor(self->m_textInfo, g_colors.player[3]);
        }
    }
}

void UIQuoridor_updatePageSettings(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    UIButton_update(self->m_buttonBack);

    UIList_update(self->m_listMode);
    UIList_update(self->m_listLevel);
    UIList_update(self->m_listCPUTime);
    UIList_update(self->m_listGridSize);
    UIList_update(self->m_listWallCount);
    UIList_update(self->m_listRandStart);
    UIList_update(self->m_listPlayerCount);

    if (UIButton_isPressed(self->m_buttonBack))
    {
        self->m_inSettings = false;
    }
}

void UIQuoridor_restartQuoridor(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);

    int wallCount = 0;
    switch (UIList_getSelected(self->m_listWallCount))
    {
    default:
    case 0: wallCount = 5; break;
    case 1: wallCount = 6; break;
    case 2: wallCount = 7; break;
    case 3: wallCount = 8; break;
    case 4: wallCount = 9; break;
    case 5: wallCount = 10; break;
    }

    int gridSize = 0;
    switch (UIList_getSelected(self->m_listGridSize))
    {
    default:
    case 0: gridSize = 5; break;
    case 1: gridSize = 7; break;
    case 2: gridSize = 9; break;
    }
    switch (UIList_getSelected(self->m_listPlayerCount))
    {
    default:
    case 0: core->playerCount = 2; break;
    case 1: core->playerCount = 4; break;
    case 2: core->playerCount = 8; break;

    }
    int tmp = 0;
    if (core->playerCount == 8)
    {
        gridSize = 17;
        tmp = 1;
    }

    QuoridorCore_reset(core, gridSize, wallCount, 0,tmp);
    if (UIList_getSelected(self->m_listRandStart) == 1)
    {
        QuoridorCore_randomStart(core);
    }

    for (int i = 0; i < 4; i++)
    {
        AIData_reset(self->m_aiData[i]); 
    }

    switch (UIList_getSelected(self->m_listPlayerCount))
    {
    default:
    case 0: core->playerCount = 2; break;
    case 1: core->playerCount = 4; break;
    case 2: core->playerCount = 8; break;

    }


    
    self->m_aiTurn.action = QUORIDOR_ACTION_UNDEFINED;
    self->m_isCheatEnable = false;
    UIQuoridor_updateRects(self);
}

void UIQuoridor_updateRects(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    Camera *camera = Scene_getCamera(self->m_scene);
    const float scale = Camera_getWorldToViewScale(camera);
    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);
    const int gridSize = core->gridSize;

    Vec2 position = { 0 };
    SDL_FRect rect = { 0 };

    const float buttonH = Text_getHeight(self->m_buttonBack->m_label) + 20.f;
    position = Vec2_set(0.25f, 8.75f);
    Camera_worldToView(camera, position, &rect.x, &rect.y);
    rect.x = roundf(rect.x);
    rect.y = roundf(rect.y);
    rect.w = 1.5f * scale;
    rect.h = buttonH;
    UIButton_setRect(self->m_buttonSettings, rect);
    UIButton_setRect(self->m_buttonBack, rect);

    self->m_rectMainPanels.x = rect.x;
    self->m_rectMainPanels.y = rect.y + rect.h;
    self->m_rectMainPanels.h = 8.50f * scale - self->m_rectMainPanels.y;
    self->m_rectMainPanels.w = 6.25f * scale;

    rect.x += roundf(rect.w + 0.2f * scale);
    UIButton_setRect(self->m_buttonRestart, rect);

    rect.x += roundf(rect.w + 0.2f * scale); 
	UIButton_setRect(self->m_buttonActionBack, rect);
    
    rect.x += roundf(rect.w + 0.2f * scale); 
	UIButton_setRect(self->m_buttonCheatSwitch, rect);


    const float labelRatio = 0.4f;
    const float settingsW = 6.0f;
    const float listSep = 10.f;
    const float titleSep = 30.f;

    UIList *lists[] = {
        self->m_listMode,
        self->m_listLevel,
        self->m_listCPUTime,
        self->m_listGridSize,
        self->m_listWallCount,
        self->m_listRandStart,
        self->m_listPlayerCount,
    };
    const int listCount = sizeof(lists) / sizeof(lists[0]);
    const float settingsH = (float)Text_getHeight(self->m_textTitleSettings)
        + listCount * (buttonH + listSep) + titleSep;

    rect.h = (float)Text_getHeight(self->m_textTitleSettings);
    rect.w = (float)Text_getWidth(self->m_textTitleSettings);
    rect.x = roundf(Camera_worldToViewX(camera, 8.f) - 0.5f * rect.w);
    rect.y = roundf(Camera_worldToViewY(camera, 4.5f) - 0.5f * settingsH);
    self->m_rectSettings = rect;

    rect.y += roundf(rect.h + titleSep);

    for (int i = 0; i < listCount; i++)
    {
        rect.x = roundf(Camera_worldToViewX(camera, 8.f - 0.5f * settingsW));
        rect.w = roundf(settingsW * scale);
        rect.h = roundf(buttonH);
        UIList_setRect(lists[i], rect, labelRatio);

        rect.y += roundf(buttonH + listSep);
    }

    const AABB worldRect = {
        .lower = Vec2_set(7.25f, 0.25f),
        .upper = Vec2_set(15.75f, 8.75f)
    };
    const float worldRectW = worldRect.upper.x - worldRect.lower.x;
    const float worldRectH = worldRect.upper.y - worldRect.lower.y;
    float wallSize = 0.03f;
    if(core->playerCount == 8)
        wallSize = 0.03f / 2;

    float cellSize = (1.f - (gridSize - 1) * wallSize) / gridSize;
    float blockSize = cellSize + wallSize;
        
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            rect.w = cellSize * scale * worldRectW;
            rect.h = cellSize * scale * worldRectH;
            position.x = j * blockSize * worldRectW;
            position.y = (1.f - i * blockSize) * worldRectH;
            position.x += worldRect.lower.x;
            position.y += worldRect.lower.y;
            Camera_worldToView(camera, position, &rect.x, &rect.y);
            self->m_rectCells[i][j] = rect;
        }
    }
    for (int i = 0; i < gridSize - 1; i++)
    {
        for (int j = 0; j < gridSize - 1; j++)
        {
            rect.w = (cellSize + blockSize) * scale * worldRectW;
            rect.h = wallSize * scale * worldRectH;
            position.x = j * blockSize * worldRectW;
            position.y = (1.f - (i * blockSize + cellSize)) * worldRectH;
            position.x += worldRect.lower.x;
            position.y += worldRect.lower.y;
            Camera_worldToView(camera, position, &rect.x, &rect.y);
            self->m_rectHWalls[i][j] = rect;
            rect.w = cellSize * scale * worldRectW;
            self->m_rectMouseHWalls[i][j] = rect;

            rect.w = wallSize * scale * worldRectW;
            rect.h = (cellSize + blockSize) * scale * worldRectH;
            position.x = (j * blockSize + cellSize) * worldRectW;
            position.y = (1.f - i * blockSize) * worldRectH;
            position.x += worldRect.lower.x;
            position.y += worldRect.lower.y;
            Camera_worldToView(camera, position, &rect.x, &rect.y);
            self->m_rectVWalls[i][j] = rect;
            rect.h = cellSize * scale * worldRectH;
            self->m_rectMouseVWalls[i][j] = rect;
        }
    }
}

void UIQuoridor_renderBoard(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    Input *input = Scene_getInput(self->m_scene);
    Vec2 mousePos = input->mousePos;
    QuoridorCore *core = Scene_getQuoridorCore(self->m_scene);
    const int gridSize = core->gridSize;

    bool playerTurn = UIQuoridor_isPlayerTurn(self);

    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            if(core->playerCount != 8)
                if ((core->state == QUORIDOR_STATE_P0_WON && j == gridSize - 1) || (core->state == QUORIDOR_STATE_P1_WON && j == 0) || (core->state == QUORIDOR_STATE_P2_WON && i == gridSize - 1) || (core->state == QUORIDOR_STATE_P3_WON && i == 0) || (core->state == QUORIDOR_STATE_P4_WON && i == gridSize - 1) || (core->state == QUORIDOR_STATE_P5_WON && i == 0) || (core->state == QUORIDOR_STATE_P6_WON && i == 0) || (core->state == QUORIDOR_STATE_P7_WON && i == 0))
                {
                    SDL_FRect* square = &self->m_rectCells[i][j];
                    square->x -= square->h / 5;
                    square->y -= square->w / 5;
                    square->h += square->h / 5;
                    square->w += square->w / 5;
                }
            if((i >= 5 && i <= gridSize - 6) || (j >= 5 && j <= gridSize - 6) || core->playerCount != 8)
            {
                Game_setRenderDrawColor(g_colors.cell, 255);
                SDL_RenderFillRect(g_renderer, &(self->m_rectCells[i][j]));
            }
            if (core->playerCount == 8)
            {
                Game_setRenderDrawColor(g_colors.back, 70);

                if (i == 5 && j < 5)
                    Game_setRenderDrawColor(g_colors.player[0], 70);
                if (i == 5 && j > gridSize - 6)
                    Game_setRenderDrawColor(g_colors.player[5], 70);
                if (i == 11 && j < 5)
                    Game_setRenderDrawColor(g_colors.player[1], 70);
                if (i == 11 && j > gridSize - 6)
                    Game_setRenderDrawColor(g_colors.player[4], 70);
                if (j == 5 && i < 5)
                    Game_setRenderDrawColor(g_colors.player[7], 70);
                if (j == 5 && i > gridSize - 6)
                    Game_setRenderDrawColor(g_colors.player[2], 70);
                if (j == 11 && i < 5)
                    Game_setRenderDrawColor(g_colors.player[6], 70);
                if (j == 11 && i > gridSize - 6)
                    Game_setRenderDrawColor(g_colors.player[3], 70);
                SDL_RenderFillRect(g_renderer, &(self->m_rectCells[i][j]));


                
            }
        }
        if(core->playerCount != 8)
        {
            Game_setRenderDrawColor(g_colors.player[1], 50);
            SDL_RenderFillRect(g_renderer, &(self->m_rectCells[i][0]));

            Game_setRenderDrawColor(g_colors.player[0], 60);
            SDL_RenderFillRect(g_renderer, &(self->m_rectCells[i][gridSize - 1]));
        }


    }
    if(core->playerCount == 4)
        for (int i = 0; i < gridSize; i++)
        {
            
            Game_setRenderDrawColor(g_colors.player[2], 50);
            SDL_RenderFillRect(g_renderer, &(self->m_rectCells[gridSize - 1][i]));
            Game_setRenderDrawColor(g_colors.player[3], 50);
            SDL_RenderFillRect(g_renderer, &(self->m_rectCells[0][i]));

        }

    int tmp = 0;
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            tmp = 0;
            QuoridorPos path[MAX_PATH_LEN];
            
#ifdef A_STAR
			int size = AStar_search(core, core->playerID, path);
#else 
			int size = BFS_search2(core, core->playerID, path);
#endif


            SDL_FRect rect = self->m_rectCells[i][j];
            for (int c = 0; c < core->playerCount; c++)
            {
                if (i == core->positions[c].i && j == core->positions[c].j && tmp == 0)
                {
                    Game_setRenderDrawColor(g_colors.player[c], 255);
                    SDL_RenderFillRect(g_renderer, &rect);
                    tmp = 1;
                    break;
                }
            }
            if (QuoridorCore_canMoveTo(core, i, j) && tmp == 0)
            {

                for (int c = 0; c < core->playerCount; c++)
                    if (core->playerID == c && tmp == 0)
                    {
                        Game_setRenderDrawColor(g_colors.player[c], 128);
                        tmp = 1;
                    }
                tmp = 1;

                rect.x += rect.w / 3; rect.y += rect.h / 3; rect.w /= 3; rect.h /= 3;

                if (path[1].i == i && path[1].j == j)
                {
                    rect.x -= rect.w / 2; rect.y -= rect.h / 2;
                    rect.w *= 2; rect.h *= 2;
                }
                SDL_RenderFillRect(g_renderer, &rect);

            }
            
            if (tmp == 0 && self->m_isCheatEnable)
            {
                for (int k = 0; k < size; k++)
                {
                    if (path[k].i == i && path[k].j == j)
                    {
                        for (int c = 0; c < core->playerCount; c++)
                            if (core->playerID == c)
                                Game_setRenderDrawColor(g_colors.player[c], 45);
                        rect.x += rect.w / 3; rect.y += rect.h / 3; rect.w /= 3; rect.h /= 3;
                        rect.x -= rect.w / 2; rect.y -= rect.h / 2;
                        rect.w *= 2; rect.h *= 2;
                        SDL_RenderFillRect(g_renderer, &rect);
                    }
                }
            }


            
            

        }
    }

    for (int i = 0; i < gridSize - 1; i++)
    {
        for (int j = 0; j < gridSize - 1; j++)
        {
            bool mouseInRect = FRect_containsPoint(&(self->m_rectMouseHWalls[i][j]), mousePos);
            if (core->hWalls[i][j] == WALL_STATE_START)
            {
                Game_setRenderDrawColor(g_colors.wall, 255);
                SDL_RenderFillRect(g_renderer, &(self->m_rectHWalls[i][j]));
            }
            else if (playerTurn && mouseInRect)
            {
                if (QuoridorCore_canPlayWall(core, WALL_TYPE_HORIZONTAL, i, j))
                {
                    Game_setRenderDrawColor(g_colors.wall, 128);
                    SDL_RenderFillRect(g_renderer, &(self->m_rectHWalls[i][j]));
                }
                else
                {
                    Game_setRenderDrawColor(g_colors.red, 128);
                    SDL_RenderFillRect(g_renderer, &(self->m_rectHWalls[i][j]));
                }
            }

            mouseInRect = FRect_containsPoint(&(self->m_rectMouseVWalls[i][j]), mousePos);
            if (core->vWalls[i][j] == WALL_STATE_START)
            {
                Game_setRenderDrawColor(g_colors.wall, 255);
                SDL_RenderFillRect(g_renderer, &(self->m_rectVWalls[i][j]));
            }
            else if (playerTurn && mouseInRect)
            {
                if (QuoridorCore_canPlayWall(core, WALL_TYPE_VERTICAL, i, j))
                {
                    Game_setRenderDrawColor(g_colors.wall, 128);
                    SDL_RenderFillRect(g_renderer, &(self->m_rectVWalls[i][j]));
                }
                else
                {
                    Game_setRenderDrawColor(g_colors.red, 128);
                    SDL_RenderFillRect(g_renderer, &(self->m_rectVWalls[i][j]));
                }
            }
        }
    }
}

void UIQuoridor_render(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    // Background
    Game_setRenderDrawColor(g_colors.back, 255);
    SDL_RenderFillRect(g_renderer, NULL);

    // Game
    if (self->m_inSettings == false)
    {
        UIQuoridor_renderPageMain(self);
        UIQuoridor_renderBoard(self);
    }
    else
    {
        UIQuoridor_renderPageSettings(self);
    }
}

void UIQuoridor_renderPageMain(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    Camera *camera = Scene_getCamera(self->m_scene);
    QuoridorCore* core = Scene_getQuoridorCore(self->m_scene);

    // UIButtons
    UIButton_render(self->m_buttonSettings);
    UIButton_render(self->m_buttonRestart);
	UIButton_render(self->m_buttonActionBack);
	UIButton_render(self->m_buttonCheatSwitch);

    // Panels
    const float blockSep = 20.f;
    float x = self->m_rectMainPanels.x;
    float y = self->m_rectMainPanels.y + blockSep;

    const float panelWidth = self->m_rectMainPanels.w;
    int tmp = core->playerCount;
    if (self->m_inSettings == false)
    {
        y = UIQuoridor_renderPanel(
            self, x, y, panelWidth,
            self->m_textTitleInfo,
            &(self->m_textInfo), 1
        );

        y += blockSep;

        y = UIQuoridor_renderPanel(
            self, x, y, panelWidth,
            self->m_textTitleWalls,
            self->m_textWalls, tmp
        );

        y += blockSep;

        y = UIQuoridor_renderPanel(
            self, x, y, panelWidth,
            self->m_textTitleDistances,
            self->m_textDistances, tmp
        );
        y += blockSep;

        y = UIQuoridor_renderPanel(
            self, x, y, panelWidth,
            self->m_textTitleTime,
            self->m_textTime, 1
        );

        y += blockSep;
        for (int p = 0; p < core->playerCount; p++)
        {
            float score = QuoridorCore_scoreNoRand(core, p) + 10;
            if (score < 0)
                score = 0;
            if (score > 20)
                score = 20;

            SDL_FRect dstRect = { 0 };
            dstRect.x = roundf(x);
            dstRect.y = roundf(y);
            dstRect.w = roundf(panelWidth);
            dstRect.h = roundf(30);
            Game_setRenderDrawColor(g_colors.cell, 255);
            SDL_RenderFillRect(g_renderer, &dstRect);
            dstRect.x = roundf(x + 4);
            dstRect.y = roundf(y + 4);
            dstRect.w = roundf((panelWidth - 8) / 20 * (score));
            dstRect.h = roundf(22);
            for (int c = 0; c < core->playerCount; c++)
                if (p == c)
                    Game_setRenderDrawColor(g_colors.player[c], 255);

            SDL_RenderFillRect(g_renderer, &dstRect);
            y += blockSep + 30;

        }


    }
}

void UIQuoridor_renderPageSettings(UIQuoridor *self)
{
    assert(self && "The UIQuoridor must be created");

    UIButton_render(self->m_buttonBack);
    UIList_render(self->m_listMode);
    UIList_render(self->m_listLevel);
    UIList_render(self->m_listCPUTime);
    UIList_render(self->m_listGridSize);
    UIList_render(self->m_listWallCount);
    UIList_render(self->m_listRandStart);
    UIList_render(self->m_listPlayerCount);

    SDL_Texture *texture = Text_getTexture(self->m_textTitleSettings);
    SDL_RenderTexture(g_renderer, texture, NULL, &(self->m_rectSettings));
}
