#ifndef CHESS_GUI_GAME_WINDOW_
#define CHESS_GUI_GAME_WINDOW_

#include "GuiFW.h"
#include "Types.h"

/** This function builds the game window, the chess game area and the side panel, and attaches
 *	all the required logic and gui components to each other. When this method is done we can start processing events
 *	to get the game going.
 */
GuiWindow* createGameWindow(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack);

#endif