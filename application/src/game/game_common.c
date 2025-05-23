/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "game/game_common.h"
#include "engine/common.h"

GameColors g_colors = {
    .black = { 0, 0, 0, 255 },
    .white = { 255, 255, 255, 255 },
    .red = { 255, 0, 0, 255 },
    .green = { 0, 255, 0, 255 },
    .blue = { 0, 0, 255, 255 },
    .yellow = { 255, 255, 0, 255 },
    .cyan = { 0, 255, 255, 255 },
    .magenta = { 255, 0, 255, 255 },
    .gold = { 225, 185, 143, 255 },
    .whiteSemi = { 255, 255, 255, 127 },
    .wall = { 199, 100, 87, 255 },
    .back = { 14, 23, 27, 255 },
    .cell = { 27, 45, 54, 255},
    .selected = { 54, 79, 89, 255 },
    .player[0] = {63, 136, 197, 255},
    .player[1] = { 231, 206, 116, 255},
    .player[2] = {246, 66, 66, 255},
    .player[3] = { 248, 155, 72, 255},
    .player[4] = { 146, 215, 104, 255},
    .player[5] = { 198, 79, 236, 255},
    .player[6] = { 79, 236, 190, 255},
    .player[7] = { 236, 76, 271, 255},

    .move[0] = { 129, 178, 217, 255},
    .move[1] = { 240, 231, 168, 255},
    .move[2] = { 0, 0, 128, 255},
    .move[3] = { 128, 0, 0, 255},
};

void Game_addAssets(AssetManager *assets)
{
    // -------------------------------------------------------------------------
    // Fonts

    AssetManager_addFont(
        assets, FONT_NORMAL, ASSETS_PATH, "font/m5x7.dat", 28
    );
    AssetManager_addFont(
        assets, FONT_BIG, ASSETS_PATH, "font/futile_pro.dat", 48
    );
}
