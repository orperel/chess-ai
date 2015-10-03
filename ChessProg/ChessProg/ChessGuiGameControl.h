#ifndef CHESS_GUI_GAME_CONTROL
#define CHESS_GUI_GAME_CONTROL

#include "GuiFW.h"
#include "Chess.h"

/** -- This unit represents manages the chess board component, gui and common logic alike. -- */

//  ------------------------------ 
//  -- Constants                --
//  ------------------------------
#define BOARD_W 480 // Board image sizes
#define BOARD_H 480

#define BUTTON_PAWN "Resources/button_pawn.bmp"
#define BUTTON_BISHOP "Resources/button_bishop.bmp"
#define BUTTON_ROOK "Resources/button_rook.bmp"
#define BUTTON_KNIGHT "Resources/button_knight.bmp"
#define BUTTON_QUEEN "Resources/button_queen.bmp"
#define BUTTON_KING "Resources/button_king.bmp"

//  ------------------------------ 
//  -- Type definitions         --
//  ------------------------------
typedef struct GameControl GameControl;

/** Information for a single square in the game control board.
 */
typedef struct
{
	int x, y; // Indices of the square
	GuiButton* chessPiece; // Pointer to the buttons representing potential chess piece and target markers on the square
	GuiButton* targetButton;
	GameControl* gameControl; // Pointer to the containing game logic struct
} GameSquare;

/** A chess board game component, composed of buttons for chess pieces (gui based), target markers and event reactions.
 *	This struct contains all logic needed to manage a chess board component.
 */
struct GameControl
{
	GameSquare gui_board[BOARD_SIZE][BOARD_SIZE]; // Store data each square on board (state and gui related controls)
	char* board; // The state of the game board

	GameSquare* selectedSquare; // A pointer to the currently selected square by the user.
								// If no square was selected, this field may be NULL.

	bool isBlackPlayerEditable; // Flags for enabling control of black / white pieces
	bool isWhitePlayerEditable;

	// Game control holds images that may be reused within (for multiple buttons as background)
	// Note: the images reside under a host panel which is responsible for releasing their memory.
	GuiImage* imgBlackPeon;
	GuiImage* imgBlackBishop;
	GuiImage* imgBlackRook;
	GuiImage* imgBlackKnight;
	GuiImage* imgBlackQueen;
	GuiImage* imgBlackKing;
	GuiImage* imgWhitePeon;
	GuiImage* imgWhiteBishop;
	GuiImage* imgWhiteRook;
	GuiImage* imgWhiteKnight;
	GuiImage* imgWhiteQueen;
	GuiImage* imgWhiteKing;
	GuiImage* imgTarget;

	// Events prompted when pieces on board are clicked
	void(*onChessPieceClick)(GuiButton* button);
	void(*onTargetClick)(GuiButton* button);

};

//  ------------------------------ 
//  -- Api functions            --
//  ------------------------------

/** Since the board is inverted on the X coordinate (rows), this helper method switches from Logic to Gui coordinates*/
int boardRowIndexToGuiRowIndex(int i);

/** Since the board is inverted on the X coordinate (rows), this helper method switches from Gui to Logic coordinates*/
int guiRowIndexToboardRowIndex(int i);

/** Creates and initializes a new game board control, representing a chess board.
 *  The board is initialized according to the state of the board parameter, and the gui component that composes
 *	the logical GameControl are added as a new branch under the hostPanel.
 */
GameControl* createGameControl(char board[BOARD_SIZE][BOARD_SIZE], GuiPanel* hostPanel,
							   void(*onChessPieceClick)(GuiButton* button),
							   void(*onTargetClick)(GuiButton* button));

/** Destroys the game control component neatly. */
void destroyGameControl(GameControl* gameControl);

/** Disables and hides all target square buttons attached to each square on the board. */
void disableAllTargetSquares(GameControl* gameControl);

/** Disables all chess pieces buttons attached to each square on the board. */
void disableAllChessPieces(GameControl* gameControl);


/** Returns the image cached in game control, that matches the chess piece of "type".
 *	Assumes GameControl has been loaded successfully.
 */
GuiImage* getImageForChessPiece(GameControl* gameControl, char type);

#endif