#ifndef CHESS_GUI_PLAYER_SELECT_WINDOW_
#define CHESS_GUI_PLAYER_SELECT_WINDOW_

#include "GuiFW.h"
#include "Types.h"

/** This function builds the player selection window, the chess game area and the side panel, and attaches
 *	all the required logic and gui components to each other. When this method is done we can start processing events
 *	to allow the player to configure the game settings (edit the board and use the buttons to select next player and
 *	game mode).
 *  For new games only, we allow editing the board.
 */
GuiWindow* createSettingsWindow(char board[BOARD_SIZE][BOARD_SIZE], bool isNewGame);

#endif