#include "GuiFW.h"
#include "Chess.h"
#include "ChessGuiCommons.h"
#include "ChessGuiGameControl.h"
#include "GameCommands.h"

int main_main(int argc, char *argv[])
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	bool isUserBlack = false;

	// Init Gui FW
	if (SDL_FAILURE_EXIT_CODE == initGui())
		exit(SDL_FAILURE_EXIT_CODE);

	GuiWindow* mainWindow = createGameWindow(board, isUserBlack);

	if (NULL == mainWindow)
		exit(GUI_ERROR_EXIT_CODE);

	int lastRenderTime = SDL_GetTicks();
	showWindow(mainWindow);

	while (!mainWindow->isWindowQuit && !processGuiEvents(mainWindow))
	{
		if (g_guiError || g_memError)
			break;

		if (SDL_GetTicks() - lastRenderTime > TIME_BETWEEN_FRAMES_MS)
		{
			showWindow(mainWindow);
			lastRenderTime = SDL_GetTicks();
		}

		if (g_guiError || g_memError)
			break;

		SDL_Delay(TIME_BETWEEN_FRAMES_MS);
	}

	// Dispose nicely of the game control and the game window
	// The game control is saved in the window's extent so it is available everywhere the window is available.
	// This way we avoid unneccessary use of globals.
	if (NULL != mainWindow->generalProperties.extent)
	{
		GameControl* gameControl = (GameControl*)mainWindow->generalProperties.extent;
		destroyGameControl(gameControl);
	}

	destroyWindow(mainWindow); // Free all gui resources, SDL is released on quitting the app

	if (g_guiError)
		return GUI_ERROR_EXIT_CODE;
	else if (g_memError)
		return MEMORY_ERROR_EXIT_CODE;
	else
		return OK_EXIT_CODE;
}
