#include <stdio.h>
#include <string.h>
#include "ChessGuiGameControl.h"

//  ------------------------------ 
//  -- Constants                --
//  ------------------------------

/** Resources images dimensions */
#define PIECE_W 100 // Common piece image dimensinos (for all chess pieces)
#define PIECE_H 160
#define SQUARE_W 51.5 // Dimensions of square on board (to space pieces)
#define SQUARE_H 51.5
#define TARGET_SQUARE_W 56 // Dimensions of "target marker" image
#define TARGET_SQUARE_H 54
#define FIRST_PIECE_OFFSET_X 35 // Top left square x pos of first piece
#define FIRST_PIECE_OFFSET_Y 0  // Top left square y pos of first piece
#define FIRST_TARGET_OFFSET_X 45  // Top left square x pos of first target square
#define FIRST_TARGET_OFFSET_Y 47  // Top left square y pos of first target square

/** Resources paths */
#define BOARD_IMG "Resources/board.bmp"
#define TARGET_SQUARE_IMG "Resources/target_square.bmp"
#define BLACK_P_IMG "Resources/black_p.bmp"
#define BLACK_B_IMG "Resources/black_b.bmp"
#define BLACK_R_IMG "Resources/black_r.bmp"
#define BLACK_N_IMG "Resources/black_n.bmp"
#define BLACK_Q_IMG "Resources/black_q.bmp"
#define BLACK_K_IMG "Resources/black_k.bmp"
#define WHITE_P_IMG "Resources/white_p.bmp"
#define WHITE_B_IMG "Resources/white_b.bmp"
#define WHITE_R_IMG "Resources/white_r.bmp"
#define WHITE_N_IMG "Resources/white_n.bmp"
#define WHITE_Q_IMG "Resources/white_q.bmp"
#define WHITE_K_IMG "Resources/white_k.bmp"

//  ------------------------------ 
//  -- Forward declarations     --
//  ------------------------------
void destroyGameControl(GameControl* gameControl);

/** Since the board is inverted on the X coordinate (rows), this helper method switches from Logic to Gui coordinates*/
int boardRowIndexToGuiRowIndex(int i)
{
	return  BOARD_SIZE - i - 1;
}

/** Since the board is inverted on the X coordinate (rows), this helper method switches from Gui to Logic coordinates*/
int guiRowIndexToboardRowIndex(int i)
{
	return  BOARD_SIZE - i - 1;
}

/** Initializes a single GameSquare within the gameControl given.
 *  i,j are the indices of the square initialized. Panel is the parent host where the square buttons reside.
 *  Returns the success status.
 */
bool initGameSquare(GameControl* gameControl, char type, int i, int j, GuiPanel* gameAreaPanel,
					void(*onChessPieceClick)(GuiButton* button),
					void(*onTargetClick)(GuiButton* button))
{
	// Add soldier piece
	GuiImage* soldierImg = NULL;

	switch (type)
	{
	case BLACK_P:
	{
		soldierImg = gameControl->imgBlackPeon;
		break;
	}
	case BLACK_B:
	{
		soldierImg = gameControl->imgBlackBishop;
		break;
	}
	case BLACK_R:
	{
		soldierImg = gameControl->imgBlackRook;
		break;
	}
	case BLACK_N:
	{
		soldierImg = gameControl->imgBlackKnight;
		break;
	}
	case BLACK_Q:
	{
		soldierImg = gameControl->imgBlackQueen;
		break;
	}
	case BLACK_K:
	{
		soldierImg = gameControl->imgBlackKing;
		break;
	}
	case WHITE_P:
	{
		soldierImg = gameControl->imgWhitePeon;
		break;
	}
	case WHITE_B:
	{
		soldierImg = gameControl->imgWhiteBishop;
		break;
	}
	case WHITE_R:
	{
		soldierImg = gameControl->imgWhiteRook;
		break;
	}
	case WHITE_N:
	{
		soldierImg = gameControl->imgWhiteKnight;
		break;
	}
	case WHITE_Q:
	{
		soldierImg = gameControl->imgWhiteQueen;
		break;
	}
	case WHITE_K:
	{
		soldierImg = gameControl->imgWhiteKing;
		break;
	}
	default:
	{
		soldierImg = NULL;
		break;
	}
	}

	Rectangle pieceBounds;
	pieceBounds.x = (int)(FIRST_PIECE_OFFSET_X + (j*SQUARE_W));
	pieceBounds.y = (int)(FIRST_PIECE_OFFSET_Y + (i*SQUARE_H));
	pieceBounds.width = PIECE_W;
	pieceBounds.height = PIECE_H;
	int piecezIndex = i + 1; // Closer rows should hide the ones behind, increase index by 1 to draw above board image

	GuiButton* soldier =
		createButton(gameAreaPanel->generalProperties.wrapper, pieceBounds, piecezIndex,
		NULL, BROWN, onChessPieceClick);
	if ((NULL == soldier) || g_guiError)
		return false;

	soldier->isEnabled = true;

	if (NULL != soldierImg) // We always create a soldier button, but set the background only if there is a piece there
		soldier->setBGImage(soldier, soldierImg); // Button uses a non owned recycleable bg image

	// Add target square
	Rectangle targetBounds;
	targetBounds.x = (int)(FIRST_TARGET_OFFSET_X + (j*SQUARE_W));
	targetBounds.y = (int)(FIRST_TARGET_OFFSET_Y + (i*SQUARE_H));
	targetBounds.width = TARGET_SQUARE_W;
	targetBounds.height = TARGET_SQUARE_H;
	int targetzIndex = 10; // Show before all other pieces
	GuiButton* target =
		createButton(gameAreaPanel->generalProperties.wrapper, targetBounds, targetzIndex,
		NULL, RED, onTargetClick);

	if ((NULL == target) || g_guiError)
		return false;

	target->isEnabled = false;
	target->generalProperties.isVisible = false;
	target->setBGImage(target, gameControl->imgTarget); // We create with non-owned target bg image that can be reused

	// Finally make sure the square is initialized in within the game control
	gameControl->gui_board[i][j].chessPiece = soldier;
	gameControl->gui_board[i][j].targetButton = target;
	gameControl->gui_board[i][j].gameControl = gameControl;
	gameControl->gui_board[i][j].x = i;
	gameControl->gui_board[i][j].y = j;

	// Keep a pointer to the square data within the button extent to make the square data available on events!
	soldier->generalProperties.extent = &gameControl->gui_board[i][j];
	target->generalProperties.extent = &gameControl->gui_board[i][j];

	return true;
}

/** Loads and create resource image for a single chess piece */
GuiImage* loadChessPieceImage(GuiPanel* hostPanel, GameControl* gameControl, const char* imgPath)
{
	Rectangle pieceBounds = { 0, 0, PIECE_W, PIECE_H };
	GuiImage* img = createImage(hostPanel->generalProperties.wrapper, pieceBounds, 0, imgPath, BROWN);
	if ((NULL == img) || g_guiError)
	{ // Clean on errors
		destroyGameControl(gameControl);
		return NULL;
	}

	img->generalProperties.isVisible = false; // Set to invisible, this is just a resource used for its surface
}

/** Creates and initializes a new game board control, representing a chess board.
 *  The board is initialized according to the state of the board parameter, and the gui component that composes
 *	the logical GameControl are added as a new branch under the hostPanel.
 */
GameControl* createGameControl(char board[BOARD_SIZE][BOARD_SIZE], GuiPanel* hostPanel,
						 	   void(*onChessPieceClick)(GuiButton* button),
							   void(*onTargetClick)(GuiButton* button))
{
	GameControl* gameControl = (GameControl*)malloc(sizeof(GameControl));
	if (NULL == gameControl)
	{
		printf("Error: creating game board control (mem allocation failed).");
		g_memError = true;
		return NULL;
	}

	gameControl->board = board;

	// Load board image
	short boardImgZIndex = 0; // Should be at the very back
	Rectangle gameAreaBounds = hostPanel->generalProperties.bounds;
	GuiImage* boardImg = createImage(hostPanel->generalProperties.wrapper, gameAreaBounds,
		boardImgZIndex, BOARD_IMG, YELLOW);
	if ((NULL == boardImg) || g_guiError)
	{ // Clean on errors
		destroyGameControl(gameControl);
		return NULL;
	}

	// Load pieces images
	gameControl->imgBlackPeon = loadChessPieceImage(hostPanel, gameControl, BLACK_P_IMG);
	if (NULL == gameControl->imgBlackPeon)
		return NULL;
	gameControl->imgBlackBishop = loadChessPieceImage(hostPanel, gameControl, BLACK_B_IMG);
	if (NULL == gameControl->imgBlackBishop)
		return NULL;
	gameControl->imgBlackRook = loadChessPieceImage(hostPanel, gameControl, BLACK_R_IMG);
	if (NULL == gameControl->imgBlackRook)
		return NULL;
	gameControl->imgBlackKnight = loadChessPieceImage(hostPanel, gameControl, BLACK_N_IMG);
	if (NULL == gameControl->imgBlackKnight)
		return NULL;
	gameControl->imgBlackQueen = loadChessPieceImage(hostPanel, gameControl, BLACK_Q_IMG);
	if (NULL == gameControl->imgBlackQueen)
		return NULL;
	gameControl->imgBlackKing = loadChessPieceImage(hostPanel, gameControl, BLACK_K_IMG);
	if (NULL == gameControl->imgBlackKing)
		return NULL;
	gameControl->imgWhitePeon = loadChessPieceImage(hostPanel, gameControl, WHITE_P_IMG);
	if (NULL == gameControl->imgWhitePeon)
		return NULL;
	gameControl->imgWhiteBishop = loadChessPieceImage(hostPanel, gameControl, WHITE_B_IMG);
	if (NULL == gameControl->imgWhiteBishop)
		return NULL;
	gameControl->imgWhiteRook = loadChessPieceImage(hostPanel, gameControl, WHITE_R_IMG);
	if (NULL == gameControl->imgWhiteRook)
		return NULL;
	gameControl->imgWhiteKnight = loadChessPieceImage(hostPanel, gameControl, WHITE_N_IMG);
	if (NULL == gameControl->imgWhiteKnight)
		return NULL;
	gameControl->imgWhiteQueen = loadChessPieceImage(hostPanel, gameControl, WHITE_Q_IMG);
	if (NULL == gameControl->imgWhiteQueen)
		return NULL;
	gameControl->imgWhiteKing = loadChessPieceImage(hostPanel, gameControl, WHITE_K_IMG);
	if (NULL == gameControl->imgWhiteKing)
		return NULL;

	// Load target image
	Rectangle targetBounds = { 0, 0, TARGET_SQUARE_W, TARGET_SQUARE_H };
	GuiImage* targetImg = createImage(hostPanel->generalProperties.wrapper, targetBounds, 0, TARGET_SQUARE_IMG, RED);
	if ((NULL == targetImg) || g_guiError)
	{ // Clean on errors
		destroyGameControl(gameControl);
		return NULL;
	}
	targetImg->generalProperties.isVisible = false; // Set to invisible, this is just a resource used for its surface
	gameControl->imgTarget = targetImg;

	// Draw each of the pieces on the board according to the board state.
	// We also place marker nodes (for moves that may be available)
	bool isError = false;
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++){
			// Remember board is inverted in memory than the way it should look on screen (inverted on rows axis)
			isError = !initGameSquare(gameControl, board[i][j],boardRowIndexToGuiRowIndex(i), j,
									  hostPanel, onChessPieceClick, onTargetClick);

			if (isError)
			{
				destroyGameControl(gameControl);
				return NULL;
			}
		}
	}

	return gameControl;
}

/** Destroys the game control component neatly. */
void destroyGameControl(GameControl* gameControl)
{
	if (gameControl == NULL)
		return;

	// All gui components exist as a branch under the host panel in the UI tree, so they are destructed with
	// the hosting window..
	// Just free the control.
	free(gameControl);
}

/** Disables and hides all target square buttons attached to each square on the board. */
void disableAllTargetSquares(GameControl* gameControl)
{
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++){
			// Disable and hide target button
			gameControl->gui_board[i][j].targetButton->isEnabled = false;
			gameControl->gui_board[i][j].targetButton->generalProperties.isVisible = false;
		}
	}
}

/** Disables all chess pieces buttons attached to each square on the board. */
void disableAllChessPieces(GameControl* gameControl)
{
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++){
			// Disable and hide target button
			gameControl->gui_board[i][j].chessPiece->isEnabled = false;
		}
	}
}