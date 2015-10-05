#ifndef CHESS_MAIN_WINDOW_
#define CHESS_MAIN_WINDOW_

#include "GuiFW.h"
#include "Types.h"

/** Sets the active window in the system. */
void setActiveWindow(GuiWindow* window);

/** Creates the main menu window of the chess game. */
GuiWindow* createMainMenu();

#endif
