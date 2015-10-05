#include <stdio.h>
#include <string.h>
#include "GuiFW.h"
#include "Chess.h"
#include "ChessGuiCommons.h"
#include "ChessGuiGameControl.h"
#include "GameCommands.h"
#include "BoardManager.h"

//  ------------------------------ 
//  -- Constants			    --
//  ------------------------------

/** Misc constants */
#define GAME_WINDOW_TITLE "Chess game"

#define BESTMOVE_BUTTON_OFFSET_Y 37 // Position of buttons
#define SAVE_BUTTON_OFFSET_Y 100
#define MENU_BUTTON_OFFSET_Y 63
#define QUIT_BUTTON_OFFSET_Y 350
#define WOODPANEL_W 160 // Dimensions for wooden side panel image
#define WOODPANEL_H WIN_H

/** Resources paths */
#define SIDE_PANEL_IMG "Resources/game_wood_panel.bmp"
#define BUTTON_BESTMOVE_IMG "Resources/button_bestmove.bmp"
#define BUTTON_SAVE_IMG "Resources/button_save.bmp"
#define BUTTON_MENU_IMG "Resources/button_mainMenu.bmp"
#define BUTTON_QUIT_IMG "Resources/button_quit.bmp"

//  ------------------------------ 
//  -- Game Logic functions     --
//  ------------------------------

/** When a chess piece is clicked, this event is prompted (it is attached to every chess piece button's onClick).
 *	We get the possible moves for this chess piece using the logic layer and mark them as possible targets on board.
 */
void onChessPieceClick(GuiButton* button)
{
	// Each chess piece caches a game square in its extent, so it can access the logic layer
	GameSquare* gameSquare = (GameSquare*)button->generalProperties.extent;

	if (NULL == gameSquare)
		return; // Avoid null extents

	GameControl* gameControl = gameSquare->gameControl;
	int guiX = boardRowIndexToGuiRowIndex(gameSquare->x); // Gui and logic indices are inverted, convert safely here
	bool isSquareBlackPiece = isSquareOccupiedByBlackPlayer(gameControl->board, guiX, gameSquare->y);
	bool isSquareWhitePiece = isSquareOccupiedByWhitePlayer(gameControl->board, guiX, gameSquare->y);

	// Abort for clicks on enemy pieces or empty squares
	if ((isSquareBlackPiece && !gameControl->isBlackPlayerEditable) ||
		(isSquareWhitePiece && !gameControl->isWhitePlayerEditable) ||
		(!isSquareBlackPiece && !isSquareWhitePiece))
			return;

	gameControl->selectedSquare = gameSquare; // Save pointer to selected square for future events
	disableAllTargetSquares(gameControl);
	Position pos = { guiX, gameSquare->y };
	LinkedList* moves = executeGetMovesForPosCommand(gameControl->board, isSquareBlackPiece, pos);

	if (moves == NULL)
		return; // Avoid memory errors

	// Activate all squares the piece can move to
	Node* currMoveNode = moves->head;
	while (NULL != currMoveNode)
	{
		Move* currMove = (Move*)currMoveNode->data;
		int logicX = guiRowIndexToboardRowIndex(currMove->nextPos.x);
		GameSquare* currSquare = &(gameControl->gui_board[logicX][currMove->nextPos.y]);
		currSquare->targetButton->isEnabled = true;
		currSquare->targetButton->generalProperties.isVisible = true;
		currMoveNode = currMoveNode->next;
	}

	deleteList(moves);
}

/** Disables all target markers and enables only chess pieces controlled by the given user (isUserBlack).
 *  This method prepares the gui board for a next user's turn.
 */
void refreshBoard(GameControl* gameControl, bool isUserBlack)
{
	gameControl->isBlackPlayerEditable = isUserBlack;
	gameControl->isWhitePlayerEditable = !isUserBlack;

	gameControl->selectedSquare = NULL;
	disableAllTargetSquares(gameControl);
	disableAllChessPieces(gameControl);

	// Enable chess pieces that can move
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++) {

			if (!isSquareOccupiedByCurrPlayer(gameControl->board, isUserBlack, i, j))
				continue; // Skip squares that don't contain the player's pieces

			Position pos = { i, j };
			LinkedList* moves = executeGetMovesForPosCommand(gameControl->board, isUserBlack, pos);
			if (g_memError)
				return;

			if (NULL != moves)
			{
				if (moves->length > 0)
				{
					int guiI = boardRowIndexToGuiRowIndex(i);
					gameControl->gui_board[guiI][j].chessPiece->isEnabled = true;
				}

				deleteList(moves);
			}
		}
	}
}

/** Shows the promotion dialog, to be used when a pawn reaches the other edge and is promoted to a new piece type.
 *	This method blocks using the dialog component until the user picks an option.
 */
char showPromotionDialog(GuiWindow* window, bool isBlackPlayer)
{
	GuiDialog* dialog = createDialog(window, BUTTON_W, BUTTON_H, DIALOG_BGIMAGE, GREEN, BLACK);
	if ((NULL == dialog) || (g_guiError))
		return;

	dialog->addOption(dialog, BUTTON_BISHOP, MAGENTA, isBlackPlayer ? BLACK_B : WHITE_B);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_ROOK, MAGENTA, isBlackPlayer ? BLACK_R : WHITE_R);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_KNIGHT, MAGENTA, isBlackPlayer ? BLACK_N : WHITE_N);
	if (g_guiError)
		return;

	dialog->addOption(dialog, BUTTON_QUEEN, MAGENTA, isBlackPlayer ? BLACK_Q : WHITE_Q);
	if (g_guiError)
		return;

	char promotion = ((char)dialog->showDialog(dialog)); // The dialog is automatically destroyed when an option is picked

	return promotion;
}

/** When a target square is clicked, this event is prompted
 *  (it is attached to every enabled target square button's onClick).
 *	We execute the given move for the selected chess piece and clicked target using the
 *	logic layer and mark them as possible targets on board.
 */
void onTargetClick(GuiButton* button)
{
	// Each target button caches a game square in its extent, so it can access the logic layer
	GameSquare* gameSquare = (GameSquare*)button->generalProperties.extent;

	if (NULL == gameSquare)
		return; // Avoid null extents

	GameControl* gameControl = gameSquare->gameControl;
	int guiStartX = boardRowIndexToGuiRowIndex(gameControl->selectedSquare->x);
	int guiTargetX = boardRowIndexToGuiRowIndex(gameSquare->x);

	// Build the move
	// Make sure to abort clicks on illegal moves
	Position initPos = { guiStartX, gameControl->selectedSquare->y };
	Position nextPos = { guiTargetX, gameSquare->y };
	Move* move = createMove(&initPos, &nextPos);
	if (g_memError)
		return;

	// If a pawn have reached the other edge, we have a promotion move.
	// Show the promotion dialog and wait for results.
	if (isSquareOnOppositeEdge(gameControl->isBlackPlayerEditable, nextPos.x) &&
		isSquareOccupiedByPawn(gameControl->board, gameControl->isBlackPlayerEditable, initPos.x, initPos.y))
	{
		move->promotion = showPromotionDialog(button->generalProperties.window, gameControl->isBlackPlayerEditable);
	}

	if (g_guiError)
		return;

	// Validate the move is legal
	bool isValidMove = validateMove(gameControl->board, gameControl->isBlackPlayerEditable, move);
	if (!isValidMove)
	{
		printf("Warning: Gui allowed user to interact with illegal move, but logic protected from executing this move.\n");
		return;
	}

	char promotion = move->promotion; // Save if this was a promotion move and its type, since after executing the move
									  // it will be destroyed.

	// Execute the move on the board
	executeMoveCommand(gameControl->board, gameControl->isBlackPlayerEditable, move);
	move = NULL; // Move have been destroyed, so we explicitly set the pointer to NULL here for readability.

	// Update the gui
	GuiImage* finalPieceImg = NULL;
	if (EMPTY != promotion)
		finalPieceImg = getImageForChessPiece(gameControl, promotion); // Promote piece
	else
		finalPieceImg = gameControl->selectedSquare->chessPiece->bgImage; // Move piece to square

	gameSquare->chessPiece->setBGImage(gameSquare->chessPiece, finalPieceImg);
	gameSquare->chessPiece->generalProperties.isVisible = true;
	gameControl->selectedSquare->chessPiece->bgImage = NULL;
	gameControl->selectedSquare->chessPiece->generalProperties.isVisible = false;
	refreshBoard(gameControl, !gameControl->isBlackPlayerEditable);
}


//  -------------------------------- 
//  -- Side Panel Logic functions --
//  --------------------------------

/** Creates a dialog, with a dynamic number of buttons, under the given window, and using the buttonImagePath as
 *  a base for all images. All images are expected to end with 1, 2, etc (the index of the next image).
 *	Images are expected to exist for all numOfButtons values.
 */
GuiDialog* createDyanmicDialog(GuiWindow* window, int numOfButtons, const char* buttonImgPath)
{
	GuiDialog* dialog = createDialog(window, BUTTON_W, BUTTON_H, DIALOG_BGIMAGE, GREEN, BLACK);
	if ((NULL == dialog) || (g_guiError))
	{
		return NULL;
	}

	// Allocate space for an image path string
	int fileImgLength = strlen(buttonImgPath) + strlen(RESOURCE_IMG_EXT) + 2;
	char* fileImg = (char*)malloc(sizeof(char)*fileImgLength);
	if (NULL == fileImg)
	{
		g_memError = true;
		return NULL;
	}

	// Create number of buttons buttons in a generic way
	char slotStr[2];
	int i;
	for (i = 1; i <= numOfButtons; i++)
	{
		// Init the string with the path of the next button image
		// It is okay to override the previous string since the image of the previous button should have been loaded
		// synchronically already.
		strcpy(fileImg, buttonImgPath);
		sprintf(slotStr, "%d", i);
		strcat(fileImg, slotStr);
		strcat(fileImg, RESOURCE_IMG_EXT);

		dialog->addOption(dialog, fileImg, MAGENTA, i);
		if (g_guiError)
		{
			g_guiError = true;
			return NULL;
		}
	}

	free(fileImg);
}

/** Marks the best move available on board. This event is prompted when the best move button is clicked. */
void onBestMoveClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
	GameControl* gameControl = (GameControl*)button->generalProperties.extent;
	int depth;

	// For AI we use the configured min max depth
	if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
	{
		depth = g_minimaxDepth;
	}
	else
	{ // for player vs player we show a dialog and let the player choose
		GuiDialog* dialog = createDyanmicDialog(window, MAX_DEPTH, MINMAX_DEPTH_IMG_PATH);
		if ((g_guiError) || (g_memError) || (NULL == dialog))
		{
			return;
		}

		// These values aren't configurable, we always know it should be lower than 1 so they are
		// a local const.
		const int bestValue = 0;
		const int cancelValue = -1;
		dialog->addOption(dialog, MINMAX_BEST_DEPTH_IMG_PATH, MAGENTA, bestValue);
		if (g_guiError)
		{
			return;
		}
		dialog->addOption(dialog, BUTTON_CANCEL_IMG, MAGENTA, cancelValue);
		if (g_guiError)
		{
			return;
		}

		// Show the dialog and get the results the user have chosen
		depth = (int)dialog->showDialog(dialog);
		if ((g_guiError) || (g_memError) || (depth == cancelValue))
		{
			return;
		}
	}

	LinkedList* bestMoves = executeGetBestMovesCommand(gameControl->board, gameControl->isBlackPlayerEditable, depth);
	if ((g_guiError) || (g_memError) || (bestMoves == NULL))
	{
		return;
	}

	// We only show one best move
	Move* move = (Move*)bestMoves->head;

}

/** Open the save game to slots dialog. This event is prompted when the save button is clicked. */
void onSaveClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
 
	GuiDialog* dialog = createDyanmicDialog(window, NUM_OF_SAVE_SLOTS, SAVE_SLOT_PATH);
	if ((g_guiError) || (g_memError) || (NULL == dialog))
	{
		return;
	}

	const int cancelValue = -1; // This value isn't configurable, we always know it should be lower than 0 so it is
								// a local const.
	dialog->addOption(dialog, BUTTON_CANCEL_IMG, MAGENTA, cancelValue);
	if (g_guiError)
	{
		return;
	}

	// Show the dialog and get the results the user have chosen
	int slotNum = (int)dialog->showDialog(dialog);
	if ((g_guiError) || (g_memError) || (slotNum == cancelValue))
	{
		return;
	}

	// Save the file to the disk. We use a predetermined file path for each slot
	char slotStr[2];
	int saveFileLength = strlen(SAVE_GAME_PATH) + strlen(SAVE_FILE_EXTENSION) + 2;
	char* saveFilePath = (char*)malloc(sizeof(char)*saveFileLength);
	if (NULL == saveFilePath)
		return;
	strcpy(saveFilePath, SAVE_GAME_PATH);
	sprintf(slotStr, "%d", slotNum);
	strcat(saveFilePath, slotStr);
	strcat(saveFilePath, SAVE_FILE_EXTENSION);

	// Execute the save command in the logic layer
	GameControl* gameControl = (GameControl*)button->generalProperties.extent;
	executeSaveCommand(gameControl->board, saveFilePath);
	free(saveFilePath);
}

/** Quits the game and returns to the main menu. This event is prompted when the menu button is clicked. */
void onMainMenuClick(GuiButton* button)
{

}

/** Quit the game entirely. This event is prompted when the quit button is clicked. */
void onQuit(GuiButton* button)
{
	// Mark window as ready to quit, this way we break the main loop and free all resources
	GuiWindow* window = (GuiWindow*)button->generalProperties.extent;
	window->isWindowQuit = true;
}

/** -- -- -- -- -- -- -- -- -- -- -- */
/** -- Gui window initialization  -- */
/** -- -- -- -- -- -- -- -- -- -- -- */

/** This function creates the game window, the chess game area and the side panel, and attaches 
 *	all the required logic and gui components to each other. When this method is done we can start processing events
 *	to get the game going.
 */
GuiWindow* createGameWindow(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	// Z indices under window
	short gameAreaPanelZIndex = 1;
	short sidePanelZIndex = 2;

	// Z indices under wooden panel
	short sidePanelImgZIndex = 0;
	short saveButtonZIndex = 1;
	short menuButtonZIndex = 2;
	short quitButtonZIndex = 3;

	GuiColorRGB bgcolor = WHITE;
	GuiWindow* mainWindow = createWindow(WIN_W, WIN_H, GAME_WINDOW_TITLE, bgcolor);

	if ((NULL == mainWindow) || g_guiError)
		return NULL; // Clean on errors

	// Side panel creation

	Rectangle sidePanelBounds = { 0, 0, WOODPANEL_W, WOODPANEL_H };
	sidePanelBounds.x = BOARD_W; // Panel is to the right of the board
	GuiPanel* sidePanel = createPanel(mainWindow->generalProperties.wrapper, sidePanelBounds, sidePanelZIndex, GREEN);
	if ((NULL == sidePanel) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	sidePanelBounds.x = 0; // Image is 0 relative to the panel
	GuiImage* sidePanelImg = createImage(sidePanel->generalProperties.wrapper, sidePanelBounds,
		sidePanelImgZIndex, SIDE_PANEL_IMG, GREEN);
	if ((NULL == sidePanelImg) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	Rectangle btnBounds = { ((WOODPANEL_W - (BUTTON_W / 2)) / 2), 0, BUTTON_W, BUTTON_H };
	btnBounds.y = BESTMOVE_BUTTON_OFFSET_Y;
	GuiButton* bestMoveBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
										  saveButtonZIndex, BUTTON_BESTMOVE_IMG, BROWN, onBestMoveClick);
	if ((NULL == bestMoveBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	btnBounds.y = SAVE_BUTTON_OFFSET_Y;
	GuiButton* saveBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		saveButtonZIndex, BUTTON_SAVE_IMG, BROWN, onSaveClick);
	if ((NULL == saveBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	btnBounds.y = MENU_BUTTON_OFFSET_Y;
	GuiButton* mainMenuBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		menuButtonZIndex, BUTTON_MENU_IMG, BROWN, onMainMenuClick);
	if ((NULL == mainMenuBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	btnBounds.y = QUIT_BUTTON_OFFSET_Y;
	GuiButton* quitBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		quitButtonZIndex, BUTTON_QUIT_IMG, BROWN, onQuit);
	if ((NULL == quitBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

	Rectangle gameAreaBounds = { 0, 0, BOARD_W, BOARD_H };
	GuiPanel* gameAreaPanel = createPanel(mainWindow->generalProperties.wrapper, gameAreaBounds, gameAreaPanelZIndex, GRAY);
	if ((NULL == gameAreaPanel) || g_guiError)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	// Game board control creation

	GameControl* gameControl = createGameControl(board, gameAreaPanel, onChessPieceClick, onTargetClick);
	if ((NULL == gameControl) || g_guiError)
	{
		destroyWindow(mainWindow);
		destroyGameControl(gameControl);
		return NULL;
	}

	refreshBoard(gameControl, isUserBlack);
	if (g_memError)
	{ // Avoid errors
		destroyWindow(mainWindow);
		destroyGameControl(gameControl);
		return NULL;
	}

	saveBtn->generalProperties.extent = gameControl; // Save a reference to the game control /window in the button extents.
													 // This makes the game control available on events.
	mainMenuBtn->generalProperties.extent = mainWindow;
	quitBtn->generalProperties.extent = mainWindow;
	mainWindow->generalProperties.extent = gameControl;

	return mainWindow;
}

int main(int argc, char *argv[])
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