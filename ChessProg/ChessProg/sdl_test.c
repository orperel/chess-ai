#include <stdio.h>
#include "GuiFW.h"

#define WIN_W 640
#define WIN_H 480
#define BOARD_W 480
#define BOARD_H 480
#define PIECE_W 91
#define PIECE_H 210

//  ------------------------------ 
//  -- App functions            --
//  ------------------------------

void onOkClick(GuiButton* button)
{
	static int pos = 0;
	int clipWidth = 458 / 5;
	int clipHeight = 758 / 8;
	Rectangle imgBounds = { pos, pos, 458, 758 };
	pos += 10;
	GuiAnimation* testAnim = createAnimation(button->generalProperties.window->generalProperties.wrapper,
		imgBounds, 0, "Resources/white_k.bmp", RED,
		clipWidth, clipHeight, 500, true, NULL);
	if (NULL == testAnim)
	{
		destroyWindow(button->generalProperties.window);
	}
}

GuiWindow* createBoard()
{
	GuiColorRGB bgcolor = { 255, 255, 255 };
	GuiWindow* mainWindow = createWindow(WIN_W, WIN_H, "Chess game", bgcolor);

	if (NULL == mainWindow)
		return NULL;

	Rectangle gameAreaBounds = { 0, 0, BOARD_W, BOARD_H };
	GuiPanel* gameAreaPanel = createPanel(mainWindow->generalProperties.wrapper, gameAreaBounds, 0, GRAY);
	if (NULL == gameAreaPanel)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	GuiImage* boardImg = createImage(gameAreaPanel->generalProperties.wrapper, gameAreaBounds,
									 50, "Resources/board.bmp", YELLOW);
	if (NULL == boardImg)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	Rectangle pieceBounds = { 0, 0, PIECE_W, PIECE_H };
	pieceBounds.x = 20;
	pieceBounds.y = 20;
	GuiButton* whiteKing = createButton(gameAreaPanel->generalProperties.wrapper, pieceBounds, 1, "Resources/white_k.bmp", GREEN, onOkClick);
	if (NULL == whiteKing)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	pieceBounds.x = 60;
	pieceBounds.y = 60;
	GuiButton* blackKing = createButton(gameAreaPanel->generalProperties.wrapper, pieceBounds, 1, "Resources/black_k.bmp", GREEN, onOkClick);
	if (NULL == blackKing)
	{
		destroyWindow(mainWindow);
		return NULL;
	}

	return mainWindow;
}

int gui_main(int argc, char *argv[])
{
	// Init Gui FW
	if (SDL_FAILURE_EXIT_CODE == initGui())
		exit(SDL_FAILURE_EXIT_CODE);

	GuiWindow* mainWindow = createBoard();

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

//int oldermain(void) {
//	SDL_Event e;
//	SDL_Rect rect = {10, 10, 50, 50};
//	SDL_Rect imgrect = {0, 0, IMG_W, IMG_H};
//	SDL_Surface *img = SDL_LoadBMP("test.bmp");
//	SDL_Surface *w  = SDL_SetVideoMode(WIN_W, WIN_H, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
//	int quit = 0;
//	
//	/* Initialize SDL and make sure it quits*/
//	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//		printf("ERROR: unable to init SDL: %s\n", SDL_GetError());
//		return 1;
//	}
//	atexit(SDL_Quit);
//
//	/* Create window surface*/
//
//	if (w == NULL) {
//		printf("ERROR: failed to set video mode: %s\n", SDL_GetError());
//		return 1;
//	}
//
//	/* Define the rects we need*/
//	
//
//	/* Load test spritesheet image*/
//	
//	if (img == NULL) {
//		printf("ERROR: failed to load image: %s\n", SDL_GetError());
//		return 1;
//	}
//
//	/* Set colorkey to BLUE*/
//	if (SDL_SetColorKey(img, SDL_SRCCOLORKEY, SDL_MapRGB(img->format, 0, 0, 255)) != 0) {
//		printf("ERROR: failed to set color key: %s\n", SDL_GetError());
//		SDL_FreeSurface(img);
//		return 1;
//	}
//
//
//	while (!quit) {
//		/* Clear window to BLACK*/
//		if (SDL_FillRect(w,0,0) != 0) {
//			printf("ERROR: failed to draw rect: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Green rectangle button*/
//		if (SDL_FillRect(w, &rect, SDL_MapRGB(w->format, 0, 255, 0)) != 0) {
//			printf("ERROR: failed to draw rect: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Draw image sprite*/
//		if (SDL_BlitSurface(img, &imgrect, w, 0) != 0) {
//			SDL_FreeSurface(img);
//			printf("ERROR: failed to blit image: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Advance to next sprite*/
//		imgrect.x += imgrect.w;
//		if (imgrect.x >= img->w) {
//			imgrect.x = 0;
//			imgrect.y += imgrect.h;
//			if (imgrect.y >= img->h) imgrect.y = 0;
//		}
//
//		/* We finished drawing*/
//		if (SDL_Flip(w) != 0) {
//			printf("ERROR: failed to flip buffer: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Poll for keyboard & mouse events*/
//
//		while (SDL_PollEvent(&e) != 0) {
//			switch (e.type) {
//				case (SDL_QUIT):
//					quit = 1;
//					break;
//				case (SDL_KEYUP):
//					if (e.key.keysym.sym == SDLK_ESCAPE) quit = 1;
//					break;
//				case (SDL_MOUSEBUTTONUP):
//					if ((e.button.x > rect.x) && (e.button.x < rect.x + rect.w) && (e.button.y > rect.y) && (e.button.y < rect.y+rect.h))
//						quit = 1;
//					break;
//				default:
//					break;
//			}
//		}
//
//		/* Wait a little before redrawing*/
//		SDL_Delay(1000);
//	}
//
//	SDL_Delay(10000);
//	SDL_FreeSurface(img);
//	return 0;
//}