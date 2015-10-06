#include "ChessMainWindow.h"
#include "ChessGuiCommons.h"
#include "ChessGuiGameControl.h"
#include "GameCommands.h"
#include "ChessGuiGameWindow.h"
#include "BoardManager.h"
#include "ChessGuiPlayerSelectWindow.h"

//  ------------------------------------------------
//  -- Type definitions, Constants & Globals      --
//  ------------------------------------------------

// Resources paths
#define BUTTON_NEW_IMG "Resources/button_newgame.bmp"
#define BUTTON_LOAD_IMG "Resources/button_load.bmp"
#define MAIN_MENU_BG_IMG "Resources/main_window_background.bmp"

// Locations for buttons on screen
#define NEW_BUTTON_OFFSET_Y 120
#define LOAD_BUTTON_OFFSET_Y 180
#define QUIT_BUTTON_OFFSET_Y 290

// A global for keeping which window is active in the system at the moment.
// We make it private here and control it via the "setActiveWindow" function.
GuiWindow* g_activeWindow = NULL;


//  --------------------------- 
//  -- Logic functions       --
//  ---------------------------

/** Starts a new game (starts the matching screen flow). This event is prompted when the new game button is clicked.
 *	(it actually takes us to the settings screen for the game)
 */
void onNewGameClick(GuiButton* button)
{
	initGlobals(); // Make sure the game settings window starts fresh
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);

	// Create new window and set it as active
	GuiWindow* settingsWindow = createSettingsWindow(board);
	if (NULL == settingsWindow)
		g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly
	
	setActiveWindow(settingsWindow); // Switch to settings window
}

/**Shows the load game dialog. This event is prompted when the load button is clicked. */
void onLoadGameClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
	char* saveFilePath = showLoadSaveDialog(window);

	if (g_guiError || g_memError || (NULL == saveFilePath))
		return;

	// Execute the load command in the logic layer
	char board[BOARD_SIZE][BOARD_SIZE];
	executeLoadCommand(board, saveFilePath);
	free(saveFilePath);
	if (g_memError)
		return;

	// Create new window and set it as active
	GuiWindow* gameWindow = createGameWindow(board, g_isNextPlayerBlack);
	if (NULL == gameWindow)
		g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly

	setActiveWindow(gameWindow); // Switch to game window
}

/** Quit the game entirely. This event is prompted when the quit button is clicked. */
void onQuitClick(GuiButton* button)
{
	// Mark window as ready to quit, this way we break the main loop and free all resources
	GuiWindow* window = (GuiWindow*)button->generalProperties.window;
	window->isWindowQuit = true;
}

/** Override destructor to enhance it and release additional resources if needed.
 *  We always call destroyWindow in the end.
 */
void destroyMainMenu(void* mainMenu)
{
	destroyWindow(mainMenu);
}

/** Creates the main menu window of the chess game. */
GuiWindow* createMainMenu()
{
	GuiWindow* mainMenu = createWindow(WIN_W, WIN_H, GAME_WINDOW_TITLE, BLACK);

	if ((NULL == mainMenu) || g_guiError || g_memError)
		return NULL; // Clean on errors

	mainMenu->generalProperties.destroy = destroyMainMenu;

	// Z order of window components
	short bgImageZOrder = 1;
	short newbuttonZOrder = 2;
	short loadButtonZOrder = 3;
	short quitButtonZOrder = 4;

	Rectangle windowBounds = { 0, 0, WIN_W, WIN_H };
	GuiImage* bgImage = createImage(mainMenu->generalProperties.wrapper, windowBounds, bgImageZOrder,
									MAIN_MENU_BG_IMG, MAGENTA);
	if ((NULL == bgImage) || g_guiError || g_memError)
	{ // Clean on errors
		destroyMainMenu(mainMenu);
		return NULL;
	}

	Rectangle btnBounds = { ((WIN_W - (BUTTON_W / 2)) / 2), 0, BUTTON_W, BUTTON_H };
	btnBounds.y = NEW_BUTTON_OFFSET_Y;
	GuiButton* newButton = createButton(mainMenu->generalProperties.wrapper, btnBounds,
		newbuttonZOrder, BUTTON_NEW_IMG, BROWN, onNewGameClick);
	if ((NULL == newButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyMainMenu(mainMenu);
		return NULL;
	}

	btnBounds.y = LOAD_BUTTON_OFFSET_Y;
	GuiButton* loadButton = createButton(mainMenu->generalProperties.wrapper, btnBounds,
		loadButtonZOrder, BUTTON_LOAD_IMG, BROWN, onLoadGameClick);
	if ((NULL == loadButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyMainMenu(mainMenu);
		return NULL;
	}

	btnBounds.y = QUIT_BUTTON_OFFSET_Y;
	GuiButton* quitButton = createButton(mainMenu->generalProperties.wrapper, btnBounds,
		quitButtonZOrder, BUTTON_QUIT_SMALL_IMG, BROWN, onQuitClick);
	if ((NULL == quitButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyMainMenu(mainMenu);
		return NULL;
	}

	return mainMenu;
}

/** Set a new window to be the active window in the applciation.
 *  Events are queried only for the active window.
 */
void setActiveWindow(GuiWindow* window)
{
	if (g_guiError || g_memError)
		return; // Don't update when the error flag is on. Protect the current existing window to prevent leaks.

	g_activeWindow = window;
}

/** The main loop of the gui, maintains a steady framerate and queries events.
 *  This loop allows the gui to be sensitive to user actions, and allows windows to switch.
 *	We always query the active window, so switching windows is easily done by calling setActiveWindow.
 *	We always draw to the screen in a framerate of 60 fps (configurable). This is important for animations
 *	and button state effects (on mouse over / click / etc animation).
 */
int runGuiMainLoop()
{
	// Init Gui FW
	if (SDL_FAILURE_EXIT_CODE == initGui())
		exit(SDL_FAILURE_EXIT_CODE);

	// Create the first screen in the game
	GuiWindow* mainMenuWindow = createMainMenu();
	setActiveWindow(mainMenuWindow);

	if (NULL == mainMenuWindow)
		exit(GUI_ERROR_EXIT_CODE);

	// Show window for the first time
	int lastRenderTime = SDL_GetTicks();
	showWindow(mainMenuWindow);

	// Helps follow when we switch windows
	GuiWindow* lastWindowProcessed = mainMenuWindow;

	// Keep event loop for the active window until we quit
	while (!g_activeWindow->isWindowQuit && !processGuiEvents(g_activeWindow))
	{
		if (g_guiError || g_memError)
			break;

		// If the active window have been switched, make sure to call the destructor of the previous window
		// to eliminate it. This is important since we want to destroy the window after all events have been processed.
		if (lastWindowProcessed != g_activeWindow)
		{
			lastWindowProcessed->generalProperties.destroy(lastWindowProcessed);
		}

		lastWindowProcessed = g_activeWindow; // Set the last window which have been processed
		if (g_guiError || g_memError)
			break;

		// Draw only when its time to draw the frame according to FPS configuration
		if (SDL_GetTicks() - lastRenderTime > TIME_BETWEEN_FRAMES_MS)
		{
			showWindow(g_activeWindow);
			lastRenderTime = SDL_GetTicks();
		}

		if (g_guiError || g_memError)
			break;

		SDL_Delay(TIME_BETWEEN_FRAMES_MS); // Maintain framerate
	}

	// Free all gui resources, SDL is released on quitting the app
	// We explicitly call the destructor here since a custom destructor for a window may override the default destroyer
	if (NULL != g_activeWindow)
		g_activeWindow->generalProperties.destroy(g_activeWindow);

	if (g_guiError)
		return GUI_ERROR_EXIT_CODE;
	else if (g_memError)
		return MEMORY_ERROR_EXIT_CODE;
	else
		return OK_EXIT_CODE;
}
