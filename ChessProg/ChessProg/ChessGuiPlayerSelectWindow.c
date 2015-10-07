#include "ChessGuiPlayerSelectWindow.h"
#include "ChessGuiCommons.h"
#include "ChessGuiGameControl.h"
#include "ChessMainWindow.h"
#include "ChessGuiGameWindow.h"
#include "ChessGuiAISettingsWindow.h"
#include "GameCommands.h"

//  -------------------------------- 
//  -- Constants and type defs    --
//  --------------------------------

#define SETTINGS_WINDOW_TITLE "Chess game :: Game Settings"

#define TITLE_BOARD_EDIT_IMG_OFFSET_Y 20 // Position of buttons
#define TITLE_BOARD_EDIT_IMG_WIDTH 150
#define TITLE_BOARD_EDIT_IMG_HEIGHT 90
#define TITLE_GAME_MODE_GAP_HEIGHT -10
#define TITLE_GAME_MODE_IMG_WIDTH 115
#define TITLE_GAME_MODE_IMG_HEIGHT 40
#define TITLE_NEXT_PLAYER_GAP_HEIGHT -10
#define TITLE_NEXT_PLAYER_IMG_WIDTH 133
#define TITLE_NEXT_PLAYER_IMG_HEIGHT 40
#define GAME_MODE_BUTTON_OFFSET_Y 170
#define NEXT_PLAYER_BUTTON_OFFSET_Y 256
#define START_BUTTON_OFFSET_Y 350
#define CANCEL_BUTTON_OFFSET_Y 400

/** Resources paths */
#define IMG_TITLE_BOARD_SETTINGS "Resources/title_board_settings.bmp"
#define IMG_TITLE_GAME_MODE "Resources/title_game_mode.bmp"
#define IMG_TITLE_NEXT_PLAYER "Resources/title_next_player.bmp"
#define BUTTON_PLAYER_VS_PLAYER_IMG "Resources/button_player_vs_player.bmp"
#define BUTTON_PLAYER_VS_AI_IMG "Resources/button_player_vs_ai.bmp"
#define BUTTON_CLEAR_IMG "Resources/button_clear.bmp"
#define BUTTON_B_PAWN "Resources/button_black_p.bmp"
#define BUTTON_B_BISHOP "Resources/button_black_b.bmp"
#define BUTTON_B_ROOK "Resources/button_black_r.bmp"
#define BUTTON_B_KNIGHT "Resources/button_black_n.bmp"
#define BUTTON_B_QUEEN "Resources/button_black_q.bmp"
#define BUTTON_B_KING "Resources/button_black_k.bmp"
#define BUTTON_W_PAWN "Resources/button_white_p.bmp"
#define BUTTON_W_BISHOP "Resources/button_white_b.bmp"
#define BUTTON_W_ROOK "Resources/button_white_r.bmp"
#define BUTTON_W_KNIGHT "Resources/button_white_n.bmp"
#define BUTTON_W_QUEEN "Resources/button_white_q.bmp"
#define BUTTON_W_KING "Resources/button_white_k.bmp"

// Message box definitions
#define MSG_INVALID_BOARD_IMG "Resources/msg_invalid_board.bmp"
#define MSG_INVALID_BOARD_W 320
#define MSG_INVALID_BOARD_H 40

/** Information attached to the settings window, to provide info in events.
 */
struct SettingsWindowExtent
{
	// Pointer to the button images that may change as user alters settings
	GuiImage* blackImg;
	GuiImage* whiteImg;
	GuiImage* playerVsPlayerImg;
	GuiImage* playerVsAi;

	GameControl* gameControl; // Pointer to the containing game logic struct
};
typedef struct SettingsWindowExtent SettingsWindowExtent;

//  -------------------------------- 
//  -- Edit board Logic functions --
//  --------------------------------

/** Initializes the board's buttons (a chess piece button is visible where we have chess pieces, empty squares get
 *  target markers).
 *  For new game - the board can be editted. For non-new games, chess pieces are visible but the board cannot be editted.
 */
void refreshSettingsBoard(GameControl* gameControl, bool isNewGame)
{
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++){

			int logicX = guiRowIndexToboardRowIndex(i);
			int logicY = j;

			// We show the correct button according to the state of the square on the logic layer board

			if (gameControl->board[logicX][logicY] == EMPTY)
			{ // Disable and hide chess piece button, 

				bool targetSquareVal = isNewGame;
				gameControl->gui_board[i][j].targetButton->isEnabled = targetSquareVal;
				gameControl->gui_board[i][j].targetButton->generalProperties.isVisible = targetSquareVal;
				gameControl->gui_board[i][j].chessPiece->isEnabled = false;
				gameControl->gui_board[i][j].chessPiece->generalProperties.isVisible = false;
			}
			else
			{ // Disable and hide target button
				gameControl->gui_board[i][j].targetButton->isEnabled = false;
				gameControl->gui_board[i][j].targetButton->generalProperties.isVisible = false;
				gameControl->gui_board[i][j].chessPiece->isEnabled = isNewGame;
				gameControl->gui_board[i][j].chessPiece->generalProperties.isVisible = true;
			}
		}
	}
}

/** Shows the chess pieces dialog, to be used when a user wants to set a new chess piece on a square.
 *	This method blocks using the dialog component until the user picks an option.
 */
void showChessPiecesDialog(GuiWindow* window, GameSquare* targetGameSquare)
{
	GuiDialog* dialog = createDialog(window, BUTTON_W, BUTTON_H, DIALOG_BGIMAGE, GREEN, BLACK);
	if ((NULL == dialog) || (g_guiError))
	{
		g_guiError = true;
		return;
	}

	// The dialog only lives inside this scope, so it is safe to pass a pointer to the local variables.
	// (the dialog values will simply point to values that live on the stack).
	// While not the safest practice, it saves redundant malloc calls here.
	char black_pawn = BLACK_P;
	char white_pawn = WHITE_P;
	char black_bishop = BLACK_B;
	char white_bishop = WHITE_B;
	char black_rook = BLACK_R;
	char white_rook = WHITE_R;
	char black_knight = BLACK_N;
	char white_knight = WHITE_N;
	char black_queen = BLACK_Q;
	char white_queen = WHITE_Q;
	char black_king = BLACK_K;
	char white_king = WHITE_K;
	char empty = EMPTY;

	dialog->addOption(dialog, BUTTON_B_PAWN, MAGENTA, &black_pawn);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_B_BISHOP, MAGENTA, &black_bishop);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_B_ROOK, MAGENTA, &black_rook);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_B_KNIGHT, MAGENTA, &black_knight);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_B_QUEEN, MAGENTA, &black_queen);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_B_KING, MAGENTA, &black_king);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_CLEAR_IMG, MAGENTA, &empty);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_PAWN, MAGENTA, &white_pawn);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_BISHOP, MAGENTA, &white_bishop);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_ROOK, MAGENTA, &white_rook);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_KNIGHT, MAGENTA, &white_knight);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_QUEEN, MAGENTA, &white_queen);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_W_KING, MAGENTA, &white_king);
	if (g_guiError)
		return;

	// The dialog is automatically destroyed when an option is picked
	// The result is the user's choice.
	char* dialogResult = (char*)dialog->showDialog(dialog);
	if (g_guiError)
		return;
	if (window->isWindowQuit)
		return; // Check if user exits the game when the dialog is open
	char piece = *(dialogResult);

	// Update the gui
	SettingsWindowExtent* extent = window->generalProperties.extent;
	GuiImage* pieceImage = getImageForChessPiece(extent->gameControl, piece);

	// Image is NULL when we clear the square
	if (NULL != pieceImage)
	{
		targetGameSquare->chessPiece->setBGImage(targetGameSquare->chessPiece, pieceImage);
		targetGameSquare->chessPiece->generalProperties.isVisible = true;
		targetGameSquare->chessPiece->isEnabled = true;
		targetGameSquare->targetButton->generalProperties.isVisible = false;
		targetGameSquare->targetButton->isEnabled = false;
	}
	else
	{
		targetGameSquare->chessPiece->bgImage = NULL;
		targetGameSquare->chessPiece->generalProperties.isVisible = false;
		targetGameSquare->chessPiece->isEnabled = false;
		targetGameSquare->targetButton->generalProperties.isVisible = true;
		targetGameSquare->targetButton->isEnabled = true;
	}

	// Update the logic layer.
	// Gui & logic indices are inverted, convert safely and update the board
	int logicX = guiRowIndexToboardRowIndex(targetGameSquare->x);
	int logicY = targetGameSquare->y;
	extent->gameControl->board[logicX][logicY] = piece;
}

/** When a chess piece is clicked, this event is prompted (it is attached to every chess piece button's onClick).
 *  Allows to set the value for the square the chess piece is on.
 */
void onSettingsChessPieceClick(GuiButton* button)
{
	// Each target button caches a game square in its extent, so it can access the logic layer
	GameSquare* gameSquare = (GameSquare*)button->generalProperties.extent;
	showChessPiecesDialog(button->generalProperties.window, gameSquare);
}

/** When a target is clicked, this event is prompted (it is attached to every chess piece button's onClick).
 *  Allows to set the value for the square the chess piece is on.
 */
void onSettingsTargetClick(GuiButton* button)
{
	// Each target button caches a game square in its extent, so it can access the logic layer
	GameSquare* gameSquare = (GameSquare*)button->generalProperties.extent;
	showChessPiecesDialog(button->generalProperties.window, gameSquare);
}


//  -------------------------------- 
//  -- Side Panel Logic functions --
//  --------------------------------

/** Opens the game mode dialog and allows the user to change the settings. */
void onGameModeClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
	SettingsWindowExtent* extent = window->generalProperties.extent;

	GuiDialog* dialog = createDialog(window, BUTTON_W, BUTTON_H, DIALOG_BGIMAGE, GREEN, BLACK);
	if ((NULL == dialog) || (g_guiError))
		return;

	// The dialog only lives inside this scope, so it is safe to pass a pointer to the local variables.
	// (the dialog values will simply point to values that live on the stack).
	// While not the safest practice, it saves redundant malloc calls here.
	int playerVsPlayer = GAME_MODE_2_PLAYERS;
	int playerVsAi = GAME_MODE_PLAYER_VS_AI;

	dialog->addOption(dialog, BUTTON_PLAYER_VS_PLAYER_IMG, MAGENTA, &playerVsPlayer);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_PLAYER_VS_AI_IMG, MAGENTA, &playerVsAi);
	if (g_guiError)
		return;

	// Update settings with the user choice
	// The dialog is automatically destroyed when an option is picked
	int* dialogResult = (int*)dialog->showDialog(dialog);
	if (g_guiError)
		return;
	if (window->isWindowQuit)
		return; // Check if user exits the game when the dialog is open
	g_gameMode = *(dialogResult);

	// Update the button look and feel to reflect the user's choice
	GuiImage* gameModeBGImg = (g_gameMode == GAME_MODE_2_PLAYERS) ? extent->playerVsPlayerImg : extent->playerVsAi;
	button->setBGImage(button, gameModeBGImg);
}

/** Opens the next player dialog and allows the user to change the settings. */
void onNextPlayerClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
	SettingsWindowExtent* extent = window->generalProperties.extent;

	bool dialogResults = showBlackWhiteDialog(window);
	if (g_guiError)
		return;
	if (window->isWindowQuit)
		return; // Check if user exits the game when the dialog is open

	g_isNextPlayerBlack = dialogResults;

	// Update the button look and feel to reflect the user's choice
	GuiImage* nextPlayerBGImg = (g_isNextPlayerBlack) ? extent->blackImg : extent->whiteImg;
	button->setBGImage(button, nextPlayerBGImg);
}

/** Starts the game if player vs player was chosen. Shows the AI settings screen if player vs AI was chosen.
 *  This event is prompted when the start button is clicked.
 */
void onStartGameClick(GuiButton* button)
{
	GuiWindow* settingsWindow = button->generalProperties.window;
	SettingsWindowExtent* settingsWindowExtent = settingsWindow->generalProperties.extent;

	bool isBoardValid = isValidStart(settingsWindowExtent->gameControl->board);

	// Show error message when board initialization is illegal
	if (!isBoardValid)
	{
		showMessageBox(settingsWindow, MSG_INVALID_BOARD_W, MSG_INVALID_BOARD_H, MSG_INVALID_BOARD_IMG, MAGENTA);
		return;
	}

	// Create new window and set it as active
	GuiWindow* nextWindow = NULL;

	if (g_gameMode == GAME_MODE_2_PLAYERS)
	{ // Player VS player continues to game screen

		GuiWindow* gameWindow = createGameWindow(settingsWindowExtent->gameControl->board, g_isNextPlayerBlack);
		if (NULL == gameWindow)
			g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly

		nextWindow = gameWindow;
	}
	else
	{ // Player VS AI continues to ai settings screen
		GuiWindow* aiWindow = createAISettingsMenu(settingsWindowExtent->gameControl->board);
		if (NULL == aiWindow)
			g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly

		nextWindow = aiWindow;
	}

	setActiveWindow(nextWindow); // Switch to game window
}

/** Quits the settings and returns to the main menu. This event is prompted when the cancel button is clicked. */
void onCancelClick(GuiButton* button)
{
	// Create new window and set it as active
	GuiWindow* mainMenu = createMainMenu();
	if (NULL == mainMenu)
		g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly

	setActiveWindow(mainMenu); // Switch to main menu window
}

/** -- -- -- -- -- -- -- -- -- -- -- */
/** -- Gui window initialization  -- */
/** -- -- -- -- -- -- -- -- -- -- -- */

/** Destroys the settings window and all objects associated with it (e.g: the game control component). */
void destroySettingsWindow(void* component)
{
	if (NULL == component)
		return;

	GuiWindow* settingsWindow = (GuiWindow*)component;

	// Dispose nicely of the game control and the settings window
	// The game control is saved in the window's extent so it is available everywhere the window is available.
	// This way we avoid unneccessary use of globals.
	if (NULL != settingsWindow->generalProperties.extent)
	{
		SettingsWindowExtent* extent = (SettingsWindowExtent*)settingsWindow->generalProperties.extent;
		GameControl* gameControl = extent->gameControl;
		destroyGameControl(gameControl);

		free(extent);
	}

	// GuiImages held by the extent are destroyed as part of the window (gui ownership)
	destroyWindow(settingsWindow);
}

/** Creates the extent object attached to the settings window and contains info useful in events.
 *
 */
SettingsWindowExtent* createSettingsWindowExtent(GuiWindow* window, GameControl* gameControl)
{
	SettingsWindowExtent* settingsWindowExtent = (SettingsWindowExtent*)malloc(sizeof(SettingsWindowExtent));
	if (NULL == settingsWindowExtent)
	{
		g_memError = true;
		g_guiError = true;
		return NULL;
	}

	settingsWindowExtent->gameControl = gameControl;

	// Create resource images (we cache them to switch the button look and feel)
	Rectangle btnBounds = { 0, 0, BUTTON_W, BUTTON_H };

	GuiImage* playerVsPlayerImg = createImage(window->generalProperties.wrapper, btnBounds,
											  0, BUTTON_PLAYER_VS_PLAYER_IMG, BROWN);
	if ((NULL == playerVsPlayerImg) || g_guiError)
	{
		return NULL;
	}

	GuiImage* playerVsAiImg = createImage(window->generalProperties.wrapper, btnBounds,
		0, BUTTON_PLAYER_VS_AI_IMG, BROWN);
	if ((NULL == playerVsAiImg) || g_guiError)
	{
		return NULL;
	}

	GuiImage* blackImg = createImage(window->generalProperties.wrapper, btnBounds,
		0, BUTTON_BLACK_IMG, BROWN);
	if ((NULL == blackImg) || g_guiError)
	{
		return NULL;
	}

	GuiImage* whiteImg = createImage(window->generalProperties.wrapper, btnBounds,
		0, BUTTON_WHITE_IMG, BROWN);
	if ((NULL == whiteImg) || g_guiError)
	{
		return NULL;
	}

	// Shared resources are invisible
	playerVsPlayerImg->generalProperties.isVisible = false;
	playerVsAiImg->generalProperties.isVisible = false;
	blackImg->generalProperties.isVisible = false;
	whiteImg->generalProperties.isVisible = false;
	settingsWindowExtent->playerVsPlayerImg = playerVsPlayerImg;
	settingsWindowExtent->playerVsAi = playerVsAiImg;
	settingsWindowExtent->blackImg = blackImg;
	settingsWindowExtent->whiteImg = whiteImg;

	return settingsWindowExtent;
}

/** This function builds the player selection window, the chess game area and the side panel, and attaches
 *	all the required logic and gui components to each other. When this method is done we can start processing events
 *	to allow the player to configure the game settings (edit the board and use the buttons to select next player and
 *	game mode).
 *  For new games only, we allow editing the board.
 */
GuiWindow* createSettingsWindow(char board[BOARD_SIZE][BOARD_SIZE], bool isNewGame)
{
	// Z indices under window
	short boardArePanelZIndex = 1;
	short sidePanelZIndex = 2;

	// Z indices under wooden panel
	// Hidden cache images are stored at the back -- z order 0
	short sidePanelImgZIndex = 1;
	short titleBoardSettingsZIndex = 2;
	short titleGameModeZIndex = 3;
	short titleNextPlayerZIndex = 4;
	short gameModeButtonZIndex = 5;
	short nextPlayerButtonZIndex = 6;
	short startButtonZIndex = 7;
	short cancelButtonZIndex = 8;

	GuiColorRGB bgcolor = WHITE;
	GuiWindow* settingsWindow = createWindow(WIN_W, WIN_H, SETTINGS_WINDOW_TITLE, bgcolor);

	if ((NULL == settingsWindow) || g_guiError)
		return NULL; // Clean on errors

	// Set a custom destructor for the window
	settingsWindow->generalProperties.destroy = destroySettingsWindow;

	// Side panel creation
	Rectangle sidePanelBounds = { 0, 0, WOODPANEL_W, WOODPANEL_H };
	sidePanelBounds.x = BOARD_W; // Panel is to the right of the board
	GuiPanel* sidePanel = createPanel(settingsWindow->generalProperties.wrapper, sidePanelBounds, sidePanelZIndex, GREEN);
	if ((NULL == sidePanel) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	sidePanelBounds.x = 0; // Image is 0 relative to the panel
	GuiImage* sidePanelImg = createImage(sidePanel->generalProperties.wrapper, sidePanelBounds,
		sidePanelImgZIndex, SIDE_PANEL_IMG, GREEN);
	if ((NULL == sidePanelImg) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	Rectangle titleBoardEditBounds = { ((WOODPANEL_W - (TITLE_BOARD_EDIT_IMG_WIDTH)) / 2),
									   TITLE_BOARD_EDIT_IMG_OFFSET_Y,
									   TITLE_BOARD_EDIT_IMG_WIDTH, TITLE_BOARD_EDIT_IMG_HEIGHT };
	GuiImage* titleBoardSettings = createImage(sidePanel->generalProperties.wrapper, titleBoardEditBounds,
											   titleBoardSettingsZIndex, IMG_TITLE_BOARD_SETTINGS, MAGENTA);
	if ((NULL == titleBoardSettings) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	Rectangle titleGameModeBounds = { ((WOODPANEL_W - (TITLE_GAME_MODE_IMG_WIDTH)) / 2),
										GAME_MODE_BUTTON_OFFSET_Y - TITLE_GAME_MODE_GAP_HEIGHT - TITLE_GAME_MODE_IMG_HEIGHT,
										TITLE_GAME_MODE_IMG_WIDTH, TITLE_GAME_MODE_IMG_HEIGHT };
	GuiImage* titleGameMode = createImage(sidePanel->generalProperties.wrapper, titleGameModeBounds,
										  titleGameModeZIndex, IMG_TITLE_GAME_MODE, MAGENTA);
	if ((NULL == titleGameMode) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	Rectangle titleNextPlayerBounds = { ((WOODPANEL_W - (TITLE_NEXT_PLAYER_IMG_WIDTH)) / 2),
										  NEXT_PLAYER_BUTTON_OFFSET_Y - TITLE_NEXT_PLAYER_GAP_HEIGHT -
										  TITLE_NEXT_PLAYER_IMG_HEIGHT,
										  TITLE_NEXT_PLAYER_IMG_WIDTH, TITLE_NEXT_PLAYER_IMG_HEIGHT };
	GuiImage* titleNextPlayer = createImage(sidePanel->generalProperties.wrapper, titleNextPlayerBounds,
		titleNextPlayerZIndex, IMG_TITLE_NEXT_PLAYER, MAGENTA);
	if ((NULL == titleNextPlayer) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	Rectangle btnBounds = { ((WOODPANEL_W - (BUTTON_W / 2)) / 2), 0, BUTTON_W, BUTTON_H };
	// Game mode and next player buttons are created with no bg image, their image state is dynamic according to settings
	btnBounds.y = GAME_MODE_BUTTON_OFFSET_Y;
	GuiButton* gameModeBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
										  gameModeButtonZIndex, NULL, BROWN, onGameModeClick);
	if ((NULL == gameModeBtn) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	btnBounds.y = NEXT_PLAYER_BUTTON_OFFSET_Y;
	GuiButton* nextPlayerBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
											nextPlayerButtonZIndex, NULL, BROWN, onNextPlayerClick);
	if ((NULL == nextPlayerBtn) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	btnBounds.y = START_BUTTON_OFFSET_Y;
	GuiButton* startBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
									   startButtonZIndex, BUTTON_START_IMG, BROWN, onStartGameClick);
	if ((NULL == startBtn) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	btnBounds.y = CANCEL_BUTTON_OFFSET_Y;
	GuiButton* cancelBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
										cancelButtonZIndex, BUTTON_CANCEL_IMG, BROWN, onCancelClick);
	if ((NULL == cancelBtn) || g_guiError)
	{ // Clean on errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	Rectangle gameAreaBounds = { 0, 0, BOARD_W, BOARD_H };
	GuiPanel* gameAreaPanel = createPanel(settingsWindow->generalProperties.wrapper,
										  gameAreaBounds, boardArePanelZIndex, GRAY);
	if ((NULL == gameAreaPanel) || g_guiError)
	{
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	// Game board control creation

	GameControl* gameControl = createGameControl(board, gameAreaPanel, onSettingsChessPieceClick, onSettingsTargetClick);
	if ((NULL == gameControl) || g_guiError)
	{
		destroyWindow(settingsWindow);
		destroyGameControl(gameControl);
		return NULL;
	}

	// Create the window extent object
	SettingsWindowExtent* windowExtent = createSettingsWindowExtent(settingsWindow, gameControl);
	if ((NULL == windowExtent) || g_memError || g_guiError)
	{ // Avoid errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	settingsWindow->generalProperties.extent = windowExtent;
	// The game mode and next player buttons are dynamic, load their initial image value from the shared resources
	// according to the settings
	GuiImage* gameModeBGImg = (g_gameMode == GAME_MODE_2_PLAYERS) ?
							windowExtent->playerVsPlayerImg : windowExtent->playerVsAi;
	GuiImage* nextPlayerBGImg = (g_isNextPlayerBlack) ? windowExtent->blackImg : windowExtent->whiteImg;
	gameModeBtn->setBGImage(gameModeBtn, gameModeBGImg);
	nextPlayerBtn->setBGImage(nextPlayerBtn, nextPlayerBGImg);

	// Initialize the visability and editability of controls within the chess component
	refreshSettingsBoard(gameControl, isNewGame);
	if (g_memError)
	{ // Avoid errors
		destroySettingsWindow(settingsWindow);
		return NULL;
	}

	// Save a reference to the settings window in the button extents.
	// This makes the game control and other useful info available on events (via the window extent).
	gameModeBtn->generalProperties.extent = settingsWindow;
	nextPlayerBtn->generalProperties.extent = settingsWindow;
	startBtn->generalProperties.extent = settingsWindow;
	cancelBtn->generalProperties.extent = settingsWindow;

	return settingsWindow;
}