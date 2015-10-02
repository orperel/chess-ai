#include <stdio.h>
#include <string.h>
#include "GuiFW.h"
#include "Chess.h"

/** Misc constants */
#define GAME_WINDOW_TITLE "Chess game"

/** Resource images dimensions */
#define WIN_W 640	// Window dimensions
#define WIN_H 480
#define BOARD_W 480 // Board image sizes
#define BOARD_H 480
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
#define BUTTON_W 200 // Dimensions for buttons
#define BUTTON_H 80
#define SAVE_BUTTON_OFFSET_Y 37 // Position of buttons
#define MENU_BUTTON_OFFSET_Y 100
#define QUIT_BUTTON_OFFSET_Y 350
#define WOODPANEL_W 160 // Dimensions for wooden side panel image
#define WOODPANEL_H BOARD_H

/** Resources paths */
#define BOARD_IMG "Resources/board.bmp"
#define SIDE_PANEL_IMG "Resources/game_wood_panel.bmp"
#define TARGET_SQUARE_IMG "Resources/target_square.bmp"
#define BUTTON_SAVE_IMG "Resources/button_save.bmp"
#define BUTTON_MENU_IMG "Resources/button_mainMenu.bmp"
#define BUTTON_QUIT_IMG "Resources/button_quit.bmp"
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
//  -- App functions            --
//  ------------------------------

void onSaveClick(GuiButton* button)
{

}

void onMainMenuClick(GuiButton* button)
{

}

void onQuit(GuiButton* button)
{

}

void onChessPieceClick(GuiButton* button)
{

}

void onTargetClick(GuiButton* button)
{

}

bool addSoldierButton(char type, int i, int j, GuiPanel* gameAreaPanel)
{
	char* soldierImgPath = NULL;
	int x, y, imgWidth, imgHeight;
	GuiColorRGB transparentColor;
	void(*onClick)(GuiButton* button);

	if (EMPTY == type)
	{
		soldierImgPath = TARGET_SQUARE_IMG;
		x = (int)(FIRST_TARGET_OFFSET_X + (j*SQUARE_W));
		y = (int)(FIRST_TARGET_OFFSET_Y + (i*SQUARE_H));
		imgWidth = TARGET_SQUARE_W;
		imgHeight = TARGET_SQUARE_H;
		transparentColor = RED;
		onClick = onTargetClick;
	}
	else
	{
		switch (type)
		{
			case BLACK_P:
			{
				soldierImgPath = BLACK_P_IMG;
				break;
			}
			case BLACK_B:
			{
				soldierImgPath = BLACK_B_IMG;
				break;
			}
			case BLACK_R:
			{
				soldierImgPath = BLACK_R_IMG;
				break;
			}
			case BLACK_N:
			{
				soldierImgPath = BLACK_N_IMG;
				break;
			}
			case BLACK_Q:
			{
				soldierImgPath = BLACK_Q_IMG;
				break;
			}
			case BLACK_K:
			{
				soldierImgPath = BLACK_K_IMG;
				break;
			}
			case WHITE_P:
			{
				soldierImgPath = WHITE_P_IMG;
				break;
			}
			case WHITE_B:
			{
				soldierImgPath = WHITE_B_IMG;
				break;
			}
			case WHITE_R:
			{
				soldierImgPath = WHITE_R_IMG;
				break;
			}
			case WHITE_N:
			{
				soldierImgPath = WHITE_N_IMG;
				break;
			}
			case WHITE_Q:
			{
				soldierImgPath = WHITE_Q_IMG;
				break;
			}
			case WHITE_K:
			{
				soldierImgPath = WHITE_K_IMG;
				break;
			}
			default: { return false; }
		}

		x = (int)(FIRST_PIECE_OFFSET_X + (j*SQUARE_W));
		y = (int)(FIRST_PIECE_OFFSET_Y + (i*SQUARE_H));
		imgWidth = PIECE_W;
		imgHeight = PIECE_H;
		transparentColor = BROWN;
		onClick = onChessPieceClick;
	}

	char* allocatedPath = (char*)malloc(strlen(soldierImgPath) + 1);
	strcpy(allocatedPath, soldierImgPath);
	Rectangle pieceBounds = { x, y, imgWidth, imgHeight };
	int zIndex = i + 1; // Closer rows should hide the ones behind, increase index by 1 to draw above board image
	GuiButton* soldier =
		createButton(gameAreaPanel->generalProperties.wrapper, pieceBounds, zIndex,
					 allocatedPath, transparentColor, onClick);
	
	return ((NULL == soldier) || g_guiError);
}

GuiWindow* createBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
	// Z indices under window
	short gameAreaPanelZIndex = 1;
	short sidePanelZIndex = 2;

	// Z indices under game area panel
	short boardImgZIndex = 0;

	// Z indices under wooden panel
	short sidePanelImgZIndex = 0;
	short saveButtonZIndex = 1;
	short menuButtonZIndex = 2;
	short quitButtonZIndex = 3;

	GuiColorRGB bgcolor = WHITE;
	GuiWindow* mainWindow = createWindow(WIN_W, WIN_H, GAME_WINDOW_TITLE, bgcolor);

	if ((NULL == mainWindow) || g_guiError)
		return NULL; // Clean on errors

	Rectangle gameAreaBounds = { 0, 0, BOARD_W, BOARD_H };
	GuiPanel* gameAreaPanel = createPanel(mainWindow->generalProperties.wrapper, gameAreaBounds, gameAreaPanelZIndex, GRAY);
	if ((NULL == gameAreaPanel) || g_guiError)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	GuiImage* boardImg = createImage(gameAreaPanel->generalProperties.wrapper, gameAreaBounds,
									 boardImgZIndex, BOARD_IMG, YELLOW);
	if ((NULL == boardImg) || g_guiError)
	{ // Clean on errors
		destroyWindow(mainWindow);
		return NULL;
	}

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

	Rectangle btnBounds = { ((WOODPANEL_W - (BUTTON_W/2)) / 2), 0, BUTTON_W, BUTTON_H };
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

	// Draw each of the pieces on the board according to the board state.
	// We also place marker nodes (for moves that may be available)
	bool isError = false;
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++){
			isError = addSoldierButton(board[i][j], i, j, gameAreaPanel);

			if (isError)
			{
				destroyWindow(mainWindow);
				return NULL;
			}
		}
	}

	return mainWindow;
}

int sdl_main(int argc, char *argv[])
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);

	// Init Gui FW
	if (SDL_FAILURE_EXIT_CODE == initGui())
		exit(SDL_FAILURE_EXIT_CODE);

	GuiWindow* mainWindow = createBoard(board);

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