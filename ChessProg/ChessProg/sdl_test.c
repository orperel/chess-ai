#include <stdio.h>
#include <string.h>
#include "GuiFW.h"
#include "Chess.h"
#include "ChessGuiGameControl.h"
#include "GameCommands.h"
#include "BoardManager.h"

/** Misc constants */
#define GAME_WINDOW_TITLE "Chess game"

/** Resource images dimensions */
#define WIN_W 640	// Window dimensions
#define WIN_H 480
#define BUTTON_W 200 // Dimensions for buttons
#define BUTTON_H 80

#define SAVE_BUTTON_OFFSET_Y 37 // Position of buttons
#define MENU_BUTTON_OFFSET_Y 100
#define QUIT_BUTTON_OFFSET_Y 350
#define WOODPANEL_W 160 // Dimensions for wooden side panel image
#define WOODPANEL_H WIN_H

/** Resources paths */
#define SIDE_PANEL_IMG "Resources/game_wood_panel.bmp"
#define BUTTON_SAVE_IMG "Resources/button_save.bmp"
#define BUTTON_MENU_IMG "Resources/button_mainMenu.bmp"
#define BUTTON_QUIT_IMG "Resources/button_quit.bmp"

//  ------------------------------ 
//  -- Logic functions          --
//  ------------------------------

void onChessPieceClick(GuiButton* button)
{
	GameSquare* gameSquare = (GameSquare*)button->generalProperties.extent;

	if (NULL == gameSquare)
		return; // Avoid null extents

	GameControl* gameControl = gameSquare->gameControl;
	int guiX = boardRowIndexToGuiRowIndex(gameSquare->x);
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

void onTargetClick(GuiButton* button)
{
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
	
	// TODO: Check for promotion and open dialog here
	if (isSquareOnOppositeEdge(gameControl->isBlackPlayerEditable, nextPos.x) &&
		isSquareOccupiedByPawn(gameControl->board, gameControl->isBlackPlayerEditable, initPos.x, initPos.y))
	{
		move->promotion = gameControl->isBlackPlayerEditable ? BLACK_Q : WHITE_Q;
	}

	bool isValidMove = validateMove(gameControl->board, gameControl->isBlackPlayerEditable, move);
	if (!isValidMove)
	{
		printf("Warning: Gui allowed user to interact with illegal move, but logic protected from executing this move.\n");
		return;
	}

	// Execute the move on the board
	executeMoveCommand(gameControl->board, gameControl->isBlackPlayerEditable, move);

	// Update the gui
	gameSquare->chessPiece->setBGImage(gameSquare->chessPiece,
									   gameControl->selectedSquare->chessPiece->bgImage);
	gameSquare->chessPiece->generalProperties.isVisible = true;
	gameControl->selectedSquare->chessPiece->bgImage = NULL;
	gameControl->selectedSquare->chessPiece->generalProperties.isVisible = false;
	refreshBoard(gameControl, !gameControl->isBlackPlayerEditable);
}

void onSaveClick(GuiButton* button)
{

}

void onMainMenuClick(GuiButton* button)
{

}

void onQuit(GuiButton* button)
{

}

/** -- -- -- -- -- -- -- -- */
/** -- Main game window  -- */
/** -- -- -- -- -- -- -- -- */

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

	return mainWindow;
}

int sdl_main(int argc, char *argv[])
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

	while (!processGuiEvents(mainWindow))
	{
		// Process logic here

		if (SDL_GetTicks() - lastRenderTime > TIME_BETWEEN_FRAMES_MS)
		{
			showWindow(mainWindow);
			lastRenderTime = SDL_GetTicks();
		}

		SDL_Delay(TIME_BETWEEN_FRAMES_MS);
	}

	return OK_EXIT_CODE;
}