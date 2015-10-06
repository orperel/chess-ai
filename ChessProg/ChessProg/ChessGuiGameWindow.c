#include "ChessGuiGameWindow.h"
#include <stdio.h>
#include <string.h>
#include "ChessGuiCommons.h"
#include "ChessGuiGameControl.h"
#include "GameCommands.h"
#include "BoardManager.h"
#include "ChessMainWindow.h"

//  ------------------------------ 
//  -- Constants			    --
//  ------------------------------

#define BESTMOVE_BUTTON_OFFSET_Y 37 // Position of buttons
#define SAVE_BUTTON_OFFSET_Y 100
#define MENU_BUTTON_OFFSET_Y 163
#define QUIT_BUTTON_OFFSET_Y 350

/** Resources paths */
#define BUTTON_BESTMOVE_IMG "Resources/button_bestmove.bmp"
#define BUTTON_SAVE_IMG "Resources/button_save.bmp"

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
 *	On error we return BLACK_K (this is an invalid promotion value).
 */
char showPromotionDialog(GuiWindow* window, bool isBlackPlayer)
{
	GuiDialog* dialog = createDialog(window, BUTTON_W, BUTTON_H, DIALOG_BGIMAGE, GREEN, BLACK);
	if ((NULL == dialog) || (g_guiError))
		return BLACK_K;

	// The dialog only lives inside this scope, so it is safe to pass a pointer to the local variables.
	// (the dialog values will simply point to values that live on the stack).
	// While not the safest practice, it saves redundant malloc calls here.
	char bishop = isBlackPlayer ? BLACK_B : WHITE_B;
	char rook = isBlackPlayer ? BLACK_R : WHITE_R;
	char knight = isBlackPlayer ? BLACK_N : WHITE_N;
	char queen = isBlackPlayer ? BLACK_Q : WHITE_Q;

	dialog->addOption(dialog, BUTTON_BISHOP, MAGENTA, &bishop);
	if (g_guiError)
		return BLACK_K;

	dialog->addOption(dialog, BUTTON_ROOK, MAGENTA, &rook);
	if (g_guiError)
		return BLACK_K;

	dialog->addOption(dialog, BUTTON_KNIGHT, MAGENTA, &knight);
	if (g_guiError)
		return BLACK_K;

	dialog->addOption(dialog, BUTTON_QUEEN, MAGENTA, &queen);
	if (g_guiError)
		return BLACK_K;

	// The dialog is automatically destroyed when an option is picked
	char* dialogResult = (char*)dialog->showDialog(dialog);
	if (g_guiError)
		return BLACK_K;
	if (window->isWindowQuit)
		return BLACK_P; // Check if user exits the game when the dialog is open
	char promotion = *(dialogResult);

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

	// Black king is an error value of showPromotionDialog (this is an invalid promotion). We quit on errors.
	if (g_guiError || g_memError || (move->promotion == BLACK_K))
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
	{ // For player vs player we show a dialog and let the player choose

		depth = showDepthDialog(window);

		// On error or cancel - return
		if (((DIFFICULTY_BEST_INT - 1) == depth) || ((DIFFICULTY_BEST_INT - 2) == depth))
			return;
	}

	LinkedList* bestMoves = executeGetBestMovesCommand(gameControl->board, gameControl->isBlackPlayerEditable, depth);
	if ((g_guiError) || (g_memError) || (bestMoves == NULL))
	{
		return;
	}

	// We only show one best move
	Move* move = (Move*)bestMoves->head->data;
	int guiStartX = boardRowIndexToGuiRowIndex(move->initPos.x);
	int guiStartY = move->initPos.y;
	int guiTargetX = boardRowIndexToGuiRowIndex(move->nextPos.x);
	int guiTargetY = move->nextPos.y;

	disableAllTargetSquares(gameControl);
	gameControl->selectedSquare = NULL;
	gameControl->gui_board[guiStartX][guiStartY].targetButton->isEnabled = false;
	gameControl->gui_board[guiTargetX][guiTargetY].targetButton->isEnabled = false;
	gameControl->gui_board[guiStartX][guiStartY].targetButton->generalProperties.isVisible = true;
	gameControl->gui_board[guiTargetX][guiTargetY].targetButton->generalProperties.isVisible = true;
}

/** Open the save game to slots dialog. This event is prompted when the save button is clicked. */
void onSaveClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;

	char* saveFilePath = showLoadSaveDialog(window);

	if (NULL == saveFilePath)
		return;

	// Execute the save command in the logic layer
	GameControl* gameControl = (GameControl*)button->generalProperties.extent;
	executeSaveCommand(gameControl->board, saveFilePath, gameControl->isBlackPlayerEditable);
	free(saveFilePath);
}

/** Quits the game and returns to the main menu. This event is prompted when the menu button is clicked. */
void onMainMenuClick(GuiButton* button)
{
	// Create new window and set it as active
	GuiWindow* mainMenu = createMainMenu();
	if (NULL == mainMenu)
		g_guiError = true; // Raise flag if an error occured, main loop will respond accordingly

	setActiveWindow(mainMenu); // Switch to main menu window
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

/** Destroys the game window and all objects associated with it (e.g: the game control component). */
void destroyGameWindow(void* component)
{
	if (NULL == component)
		return;

	GuiWindow* gameWindow = (GuiWindow*)component;

	// Dispose nicely of the game control and the game window
	// The game control is saved in the window's extent so it is available everywhere the window is available.
	// This way we avoid unneccessary use of globals.
	if (NULL != gameWindow->generalProperties.extent)
	{
		GameControl* gameControl = (GameControl*)gameWindow->generalProperties.extent;
		destroyGameControl(gameControl);
	}

	destroyWindow(gameWindow);
}

/** This function builds the game window, the chess game area and the side panel, and attaches
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
	GuiWindow* gameWindow = createWindow(WIN_W, WIN_H, GAME_WINDOW_TITLE, bgcolor);

	if ((NULL == gameWindow) || g_guiError)
		return NULL; // Clean on errors

	// Set a custom destructor for the window
	gameWindow->generalProperties.destroy = destroyGameWindow;

	// Side panel creation

	Rectangle sidePanelBounds = { 0, 0, WOODPANEL_W, WOODPANEL_H };
	sidePanelBounds.x = BOARD_W; // Panel is to the right of the board
	GuiPanel* sidePanel = createPanel(gameWindow->generalProperties.wrapper, sidePanelBounds, sidePanelZIndex, GREEN);
	if ((NULL == sidePanel) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	sidePanelBounds.x = 0; // Image is 0 relative to the panel
	GuiImage* sidePanelImg = createImage(sidePanel->generalProperties.wrapper, sidePanelBounds,
		sidePanelImgZIndex, SIDE_PANEL_IMG, GREEN);
	if ((NULL == sidePanelImg) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	Rectangle btnBounds = { ((WOODPANEL_W - (BUTTON_W / 2)) / 2), 0, BUTTON_W, BUTTON_H };
	btnBounds.y = BESTMOVE_BUTTON_OFFSET_Y;
	GuiButton* bestMoveBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		saveButtonZIndex, BUTTON_BESTMOVE_IMG, BROWN, onBestMoveClick);
	if ((NULL == bestMoveBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	btnBounds.y = SAVE_BUTTON_OFFSET_Y;
	GuiButton* saveBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		saveButtonZIndex, BUTTON_SAVE_IMG, BROWN, onSaveClick);
	if ((NULL == saveBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	btnBounds.y = MENU_BUTTON_OFFSET_Y;
	GuiButton* mainMenuBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		menuButtonZIndex, BUTTON_MENU_IMG, BROWN, onMainMenuClick);
	if ((NULL == mainMenuBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	btnBounds.y = QUIT_BUTTON_OFFSET_Y;
	GuiButton* quitBtn = createButton(sidePanel->generalProperties.wrapper, btnBounds,
		quitButtonZIndex, BUTTON_QUIT_IMG, BROWN, onQuit);
	if ((NULL == quitBtn) || g_guiError)
	{ // Clean on errors
		destroyWindow(gameWindow);
		return NULL;
	}

	Rectangle gameAreaBounds = { 0, 0, BOARD_W, BOARD_H };
	GuiPanel* gameAreaPanel = createPanel(gameWindow->generalProperties.wrapper, gameAreaBounds, gameAreaPanelZIndex, GRAY);
	if ((NULL == gameAreaPanel) || g_guiError)
	{
		destroyWindow(gameWindow);
		return NULL;
	}

	// Game board control creation

	GameControl* gameControl = createGameControl(board, gameAreaPanel, onChessPieceClick, onTargetClick);
	if ((NULL == gameControl) || g_guiError)
	{
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}

	refreshBoard(gameControl, isUserBlack);
	if (g_memError)
	{ // Avoid errors
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}

	saveBtn->generalProperties.extent = gameControl; // Save a reference to the game control /window in the button extents.
	// This makes the game control available on events.
	bestMoveBtn->generalProperties.extent = gameControl;
	mainMenuBtn->generalProperties.extent = gameWindow;
	quitBtn->generalProperties.extent = gameWindow;
	gameWindow->generalProperties.extent = gameControl;

	return gameWindow;
}