#ifndef CHESS_GUI_AI_SETTINGS_WINDOW_
#define CHESS_GUI_AI_SETTINGS_WINDOW_

#include "GuiFW.h"
#include "Types.h"
#include "ChessGuiGameControl.h"

/** Creates the AI settings window of the chess game. */
GuiWindow* createAISettingsMenu(char board[BOARD_SIZE][BOARD_SIZE]);

#endif