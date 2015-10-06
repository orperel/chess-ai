#ifndef CHESS_MAIN_WINDOW_
#define CHESS_MAIN_WINDOW_

#include "GuiFW.h"
#include "Types.h"

/** Sets the active window in the system. */
void setActiveWindow(GuiWindow* window);

GuiWindow* createMainMenu();

/** The main loop of the gui, maintains a steady framerate and queries events.
 *  This loop allows the gui to be sensitive to user actions, and allows windows to switch.
 *	We always query the active window, so switching windows is easily done by calling setActiveWindow.
 *	We always draw to the screen in a framerate of 60 fps (configurable). This is important for animations
 *	and button state effects (on mouse over / click / etc animation).
 */
int runGuiMainLoop();

#endif
