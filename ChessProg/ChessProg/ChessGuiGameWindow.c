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

#define AI_THINKING_FREEZE_TIME 150 // Amount of time in milliseconds we wait for the computer to make turn, so the
									// user can see it executing a move..

#define STATE_MESSAGE_APPERANCE_TIME 1000 // Time it takes for messages like "CHECK" to dissappear (in ms)

#define BESTMOVE_BUTTON_OFFSET_Y 37 // Position of buttons
#define SAVE_BUTTON_OFFSET_Y 100
#define MENU_BUTTON_OFFSET_Y 163
#define QUIT_BUTTON_OFFSET_Y 350

/** Resources paths */
#define BUTTON_BESTMOVE_IMG "Resources/button_bestmove.bmp"
#define BUTTON_SAVE_IMG "Resources/button_save.bmp"
#define IMG_TIE "Resources/image_tie.bmp"
#define IMG_CHECK "Resources/image_check.bmp"
#define IMG_MATE_BLACK_WINS "Resources/image_mate_black_wins.bmp"
#define IMG_MATE_WHITE_WINS "Resources/image_mate_white_wins.bmp"
#define STATE_IMG_HEIGHT 480
#define STATE_IMG_WIDTH 480

/** Information attached to the game window, to provide info in events.
*/
struct GameWindowExtent
{

	// Pointer to the button images that may change as user alters settings
	GuiImage* checkImg;
	GuiImage* mateBlackWinsImg;
	GuiImage* mateWhiteWinsImg;
	GuiImage* tieImg;
	GuiButton* bestMoveButton;
	GameControl* gameControl;
};
typedef struct GameWindowExtent GameWindowExtent;

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
void refreshBoard(GameControl* gameControl)
{
	gameControl->isBlackPlayerEditable = (g_isNextPlayerBlack &&
										  ((g_gameMode == GAME_MODE_2_PLAYERS) ||
										  ((g_gameMode == GAME_MODE_PLAYER_VS_AI) && g_isUserBlack)));
	gameControl->isWhitePlayerEditable = (!g_isNextPlayerBlack &&
										  ((g_gameMode == GAME_MODE_2_PLAYERS) ||
										  ((g_gameMode == GAME_MODE_PLAYER_VS_AI) && !g_isUserBlack)));

	gameControl->selectedSquare = NULL;
	disableAllTargetSquares(gameControl);
	disableAllChessPieces(gameControl);

	// The board can't be editted by the user
	if (!gameControl->isBlackPlayerEditable && !gameControl->isWhitePlayerEditable)
		return;

	// Enable chess pieces that can move
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++) {

			if (!isSquareOccupiedByCurrPlayer(gameControl->board, gameControl->isBlackPlayerEditable, i, j))
				continue; // Skip squares that don't contain the player's pieces

			Position pos = { i, j };
			LinkedList* moves = executeGetMovesForPosCommand(gameControl->board, gameControl->isBlackPlayerEditable, pos);
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

/** Refresh and update the gui board after a move has been executed. */
void updateGuiAfterMove(GameControl* gameControl, char promotion, GameSquare* source, GameSquare* target)
{
	// Update the gui
	GuiImage* finalPieceImg = NULL;
	if (EMPTY != promotion)
		finalPieceImg = getImageForChessPiece(gameControl, promotion); // Promote piece
	else
		finalPieceImg = source->chessPiece->bgImage; // Move piece to square

	target->chessPiece->setBGImage(target->chessPiece, finalPieceImg);
	target->chessPiece->generalProperties.isVisible = true;
	source->chessPiece->bgImage = NULL;
	source->chessPiece->generalProperties.isVisible = false;
	refreshBoard(gameControl);
}

/** Execute the move in the logic layer, check for the game state (check, mate, etc) and update the gui accordingly.
 *	Returns if the game is over.
 */
bool executeGuiTurn(GuiWindow* window, GameControl* gameControl, Move* move)
{
	char promotion = move->promotion;
	Position sourcePos = move->initPos;
	Position targetePos = move->nextPos;
	int guiSourceX = boardRowIndexToGuiRowIndex(sourcePos.x);
	int guiTargetX = boardRowIndexToGuiRowIndex(targetePos.x);
	GameSquare* sourceSquare = &(gameControl->gui_board[guiSourceX][sourcePos.y]);
	GameSquare* targetSquare = &(gameControl->gui_board[guiTargetX][targetePos.y]);

	// Execute the move on the board
	if (!executeMoveCommand(gameControl->board, move))
		g_guiError = true;

	move = NULL; // Move have been destroyed, so we explicitly set the pointer to NULL here for readability.

	g_isNextPlayerBlack = !g_isNextPlayerBlack; // Switch to next player

	// Check for check / mate / tie
	ChessGameState gameState = executeCheckMateTieCommand(gameControl->board, g_isNextPlayerBlack);
	bool isGameOver = false;
	GuiImage* stateImage = NULL;
	GameWindowExtent* gameWindowExtent = (GameWindowExtent*)window->generalProperties.extent;

	switch (gameState)
	{
		case(GAME_MATE_BLACK_WINS) :
		{
			stateImage = gameWindowExtent->mateBlackWinsImg;
			isGameOver = true;
			break;
		}
		case(GAME_MATE_WHITE_WINS) :
		{
			stateImage = gameWindowExtent->mateWhiteWinsImg;
			isGameOver = true;
			break;
		}
		case(GAME_CHECK) :
		{
			stateImage = gameWindowExtent->checkImg;
			isGameOver = false;
			break;
		}
		case(GAME_TIE) :
		{
			stateImage = gameWindowExtent->tieImg;
			isGameOver = true;
			break;
		}
		case(GAME_ERROR) :
		{
			isGameOver = true;
			break;
		}
		default:
		{
			isGameOver = false;
			break;
		}
	}

	// Show state image
	if (stateImage != NULL)
	{
		stateImage->generalProperties.isVisible = true;
	}

	// Redraw the frame, so we immediately see the temporary state image
	showWindow(window);

	// If the game is not over, show the state image for a short period
	if ((stateImage != NULL) && (!isGameOver))
	{
		gui_delay(STATE_MESSAGE_APPERANCE_TIME);
		stateImage->generalProperties.isVisible = false;
	}

	// Update the gui to reflect the most recent move
	updateGuiAfterMove(gameControl, promotion, sourceSquare, targetSquare);

	// If it is over, disable the board
	if (isGameOver)
	{
		disableAllChessPieces(gameControl);
		disableAllTargetSquares(gameControl);
		gameWindowExtent->bestMoveButton->isEnabled = false;
	}

	// Redraw the frame, so if the AI thinks on the next turn we immediately see the results
	showWindow(window);

	return isGameOver;
}

/** Executes the next turn by the computer. */
void executeGuiNextComputerMove(GuiWindow* gameWindow)
{
	GameWindowExtent* windowExtent = (GameWindowExtent*)gameWindow->generalProperties.extent;
	GameControl* gameControl = windowExtent->gameControl;

	gui_delay(AI_THINKING_FREEZE_TIME); // Wait minimal amount of time for computer turn
	Move* nextComputerMove = executeGetNextComputerMoveCommand(gameControl->board, g_isUserBlack);
	if (NULL == nextComputerMove)
	{
		g_guiError = true;
		return;
	}

	// Execute the move and update the gui
	executeGuiTurn(gameWindow, gameControl, nextComputerMove);
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
	if ((g_memError) || (NULL == move))
		return;

	// If a pawn have reached the other edge, we have a promotion move.
	// Show the promotion dialog and wait for results.
	// Since this event can only be prompted by a player, we can count on the isBlackPlayerEditable property
	if (isSquareOnOppositeEdge(gameControl->isBlackPlayerEditable, nextPos.x) &&
		isSquareOccupiedByPawn(gameControl->board, gameControl->isBlackPlayerEditable, initPos.x, initPos.y))
	{
		move->promotion = showPromotionDialog(button->generalProperties.window, gameControl->isBlackPlayerEditable);
	}

	// Black king is an error value of showPromotionDialog (this is an invalid promotion). We quit on errors.
	if (g_guiError || g_memError || (move->promotion == BLACK_K))
	{
		deleteMove(move);
		return;
	}

	// Validate the move is legal
	bool isValidMove = validateMove(gameControl->board, gameControl->isBlackPlayerEditable, move);
	if (!isValidMove)
	{
		printf("Warning: Gui allowed user to interact with illegal move, but logic protected from executing this move.\n");
		deleteMove(move);
		return;
	}

	GuiWindow* window = (GuiWindow*)button->generalProperties.window;

	// Execute the move and update the gui. If the game is over, return.
	bool isGameOver = executeGuiTurn(window, gameControl, move);
	if (isGameOver)
		return;

	// When playing against the AI, execute the next turn
	if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
		executeGuiNextComputerMove(window);
}

//  -------------------------------- 
//  -- Side Panel Logic functions --
//  --------------------------------

/** Marks the best move available on board. This event is prompted when the best move button is clicked. */
void onBestMoveClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;
	GameWindowExtent* windowExtent = (GameWindowExtent*)window->generalProperties.extent;
	GameControl* gameControl = windowExtent->gameControl;
	int depth;

	// For AI we use the configured min max depth
	if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
	{
		depth = g_minimaxDepth;
	}
	else
	{ // For player vs player we show a dialog and let the player choose

		depth = showDepthDialog(window);
		if (g_guiError)
			return;
		if (window->isWindowQuit)
			return; // Check if user exits the game when the dialog is open
		if (((DIFFICULTY_BEST_INT - 1) == depth) || ((DIFFICULTY_BEST_INT - 2) == depth))
			return; // On error or cancel - return
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

	deleteList(bestMoves);
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
	executeSaveCommand(gameControl->board, saveFilePath, g_isNextPlayerBlack);
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

/** Triggered when the window is showen for the first time */
void onGameWindowShow(GuiWindow* window)
{
	// If we're playing against the AI and the AI starts
	if ((g_gameMode == GAME_MODE_PLAYER_VS_AI) && (g_isUserBlack != g_isNextPlayerBlack))
	{
		executeGuiNextComputerMove(window);
	}
}

/** Destroys the game window and all objects associated with it (e.g: the game control component). */
void destroyGameWindow(void* component)
{
	if (NULL == component)
		return;

	GuiWindow* gameWindow = (GuiWindow*)component;

	// Dispose nicely of the game control and the game window
	// The game control is saved in the game window's extent so it is available everywhere the window is available.
	// This way we avoid unneccessary use of globals.
	if (NULL != gameWindow->generalProperties.extent)
	{
		GameWindowExtent* extent = (GameWindowExtent*)gameWindow->generalProperties.extent;
		
		if (NULL != extent->gameControl)
			destroyGameControl(extent->gameControl);
		
		free(extent);
	}

	destroyWindow(gameWindow);
}

/** Creates the game window extent, containing additional data about the game window (e.g: game control, quick
 *	access to components).
 */
GameWindowExtent* createGameWindowExtent(GuiWindow* gameWindow, GuiPanel* gameAreaPanel, GuiButton* bestButton,
										 char board[BOARD_SIZE][BOARD_SIZE])
{
	GameWindowExtent* gameWindowExtent = (GameWindowExtent*)malloc(sizeof(GameWindowExtent));
	if (NULL == gameWindowExtent)
	{
		g_memError = true;
		g_guiError = true;
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

	gameWindowExtent->gameControl = gameControl;

	// Z indices under board
	short stateImagesZIndex = 9000;

	Rectangle stateImgBounds = { 0, 0, STATE_IMG_WIDTH, STATE_IMG_HEIGHT };
	GuiImage* tieImg = createImage(gameAreaPanel->generalProperties.wrapper, stateImgBounds,
		stateImagesZIndex, IMG_TIE, MAGENTA);
	if ((NULL == tieImg) || g_guiError)
	{
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}
	tieImg->generalProperties.isVisible = false;

	GuiImage* checkImg = createImage(gameAreaPanel->generalProperties.wrapper, stateImgBounds,
		stateImagesZIndex, IMG_CHECK, MAGENTA);
	if ((NULL == checkImg) || g_guiError)
	{
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}
	checkImg->generalProperties.isVisible = false;

	GuiImage* mateBlackWinsImg = createImage(gameAreaPanel->generalProperties.wrapper, stateImgBounds,
		stateImagesZIndex, IMG_MATE_BLACK_WINS, MAGENTA);
	if ((NULL == mateBlackWinsImg) || g_guiError)
	{
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}
	mateBlackWinsImg->generalProperties.isVisible = false;

	GuiImage* mateWhiteWinsImg = createImage(gameAreaPanel->generalProperties.wrapper, stateImgBounds,
		stateImagesZIndex, IMG_MATE_WHITE_WINS, MAGENTA);
	if ((NULL == mateWhiteWinsImg) || g_guiError)
	{
		destroyWindow(gameWindow);
		destroyGameControl(gameControl);
		return NULL;
	}
	mateWhiteWinsImg->generalProperties.isVisible = false;

	// Save components for quick access in the end of the game
	gameWindowExtent->checkImg = checkImg;
	gameWindowExtent->mateBlackWinsImg = mateBlackWinsImg;
	gameWindowExtent->mateWhiteWinsImg = mateWhiteWinsImg;
	gameWindowExtent->tieImg = tieImg;
	gameWindowExtent->bestMoveButton = bestButton;

	return gameWindowExtent;
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

	// Create the game window extent
	GameWindowExtent* windowExtent = createGameWindowExtent(gameWindow, gameAreaPanel, bestMoveBtn, board);
	if (NULL == windowExtent)
	{
		gameWindow->generalProperties.destroy(gameWindow);
		return NULL;
	}
	gameWindow->generalProperties.extent = windowExtent;
	GameControl* gameControl = windowExtent->gameControl;

	refreshBoard(gameControl);
	if (g_memError)
	{ // Avoid errors
		gameWindow->generalProperties.destroy(gameWindow);
		return NULL;
	}

	saveBtn->generalProperties.extent = gameControl; // Save a reference to the game control / window in the button extents.
	// This makes the game control available on events.
	bestMoveBtn->generalProperties.extent = gameControl;
	mainMenuBtn->generalProperties.extent = gameWindow;
	quitBtn->generalProperties.extent = gameWindow;

	gameWindow->onShow = onGameWindowShow; // Set the onShow event, so when the window is drawn we can start playing

	return gameWindow;
}