#include "ChessGuiAISettingsWindow.h"
#include "ChessGuiCommons.h"
#include "ChessGuiGameWindow.h"
#include "ChessMainWindow.h"
#include "GameCommands.h"

//  ------------------------------------------------
//  -- Type definitions, Constants & Globals      --
//  ------------------------------------------------

#define AI_WINDOW_TITLE "Chess game :: AI Settings"

// Resources paths
#define AI_BACKGROUND_IMG "Resources/ai_window_background.bmp"
#define TITLE_USER_COLOR_IMG "Resources/title_user_color.bmp"
#define TITLE_AI_LEVEL_IMG "Resources/title_ai_difficulty.bmp"
#define TITLE_USER_COLOR_GAP_HEIGHT -10
#define TITLE_USER_COLOR_IMG_WIDTH 115
#define TITLE_USER_COLOR_IMG_HEIGHT 40
#define TITLE_AI_LEVEL_GAP_HEIGHT -10
#define TITLE_AI_LEVEL_IMG_WIDTH 123
#define TITLE_AI_LEVEL_IMG_HEIGHT 40

// Locations for buttons on screen
#define USER_COLOR_BUTTON_OFFSET_Y 140
#define AI_LEVEL_BUTTON_OFFSET_Y 220
#define START_BUTTON_OFFSET_Y 330

/** Information attached to the ai window, to provide info in events.
 */
struct AIWindowExtent
{
	// Pointer to the button images that may change as user alters settings
	GuiImage* blackImg;
	GuiImage* whiteImg;
	GuiImage* depthImg[MAX_DEPTH];
	GuiImage* bestDepthImg;

	GameControl* gameControl; // Pointer to the containing game logic struct
};
typedef struct AIWindowExtent AIWindowExtent;

//  --------------------------- 
//  -- Logic functions       --
//  ---------------------------

void onUserColorClick(GuiButton* button)
{

}

void onAILevelClick(GuiButton* button)
{
	GuiWindow* window = button->generalProperties.window;

	int depth = showDepthDialog(window);

	// On error or cancel - return
	if (((DIFFICULTY_BEST_INT - 1) == depth) || ((DIFFICULTY_BEST_INT - 2) == depth) || g_guiError || g_memError)
		return;

	if (DIFFICULTY_BEST_INT == depth)
	{

	}
	else
	{

	}

	//g_isDifficultyBest = (depth == )
}

void onStartClick(GuiButton* button)
{

}

AIWindowExtent* createAIWindowExtent(GuiWindow* window, GameControl* gameControl)
{
	AIWindowExtent* windowExtent = (AIWindowExtent*)malloc(sizeof(AIWindowExtent));
	if (NULL == windowExtent)
	{
		g_memError = true;
		g_guiError = true;
		return NULL;
	}

	windowExtent->gameControl = gameControl;

	// Create resource images (we cache them to switch the button look and feel)
	Rectangle btnBounds = { 0, 0, BUTTON_W, BUTTON_H };

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

	GuiImage* bestImg = createImage(window->generalProperties.wrapper, btnBounds,
		0, MINMAX_BEST_DEPTH_IMG_PATH, BROWN);
	if ((NULL == bestImg) || g_guiError)
	{
		return NULL;
	}

	// Shared resources are invisible
	bestImg->generalProperties.isVisible = false;
	blackImg->generalProperties.isVisible = false;
	whiteImg->generalProperties.isVisible = false;
	windowExtent->bestDepthImg = bestImg;
	windowExtent->blackImg = blackImg;
	windowExtent->whiteImg = whiteImg;

	// We load a dynamic number of depth images
	// Allocate space for an depth image path string
	int fileImgLength = strlen(MINMAX_DEPTH_IMG_PATH) + strlen(RESOURCE_IMG_EXT) + 2; // Leave space for /0 and depth index
	char* fileImg = (char*)malloc(sizeof(char)*fileImgLength);
	if (NULL == fileImg)
	{
		g_memError = true;
		return NULL;
	}

	// Create number of buttons in a generic way
	char slotStr[2];
	int i;
	for (i = 1; i <= MAX_DEPTH; i++)
	{
		// Get next depth image
		strcpy(fileImg, MINMAX_DEPTH_IMG_PATH);
		sprintf(slotStr, "%d", i);
		strcat(fileImg, slotStr);
		strcat(fileImg, RESOURCE_IMG_EXT);

		GuiImage* depthImage = createImage(window->generalProperties.wrapper, btnBounds,
			0, fileImg, BROWN);
		if ((NULL == depthImage) || g_guiError)
		{
			free(fileImg);
			return NULL;
		}

		depthImage->generalProperties.isVisible = false;
		windowExtent->depthImg[i - 1] = depthImage;
	}

	free(fileImg);

	return windowExtent;
}

/** Override destructor to enhance it and release additional resources if needed.
 *  We always call destroyWindow in the end.
 */
void destroyAIWindow(void* mainMenu)
{
	// All image resources in the extent are destroyed as part of the window (Gui ownership)
	destroyWindow(mainMenu);
}

/** Creates the AI settings window of the chess game. */
GuiWindow* createAISettingsMenu()
{
	GuiWindow* aiWindow = createWindow(WIN_W, WIN_H, AI_WINDOW_TITLE, BLACK);

	if ((NULL == aiWindow) || g_guiError || g_memError)
		return NULL; // Clean on errors

	aiWindow->generalProperties.destroy = destroyAIWindow;

	// Z order of window components
	short bgImageZOrder = 1;
	short userColorTitleZOrder = 2;
	short aiLevelTitleZOrder = 3;
	short userColorButtonZOrder = 4;
	short aiLevelButtonZOrder = 5;
	short startButtonZOrder = 6;

	Rectangle windowBounds = { 0, 0, WIN_W, WIN_H };
	GuiImage* bgImage = createImage(aiWindow->generalProperties.wrapper, windowBounds, bgImageZOrder,
									AI_BACKGROUND_IMG, MAGENTA);
	if ((NULL == bgImage) || g_guiError || g_memError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	Rectangle btnBounds = { ((WIN_W - (BUTTON_W / 2)) / 2), 0, BUTTON_W, BUTTON_H };
	btnBounds.y = USER_COLOR_BUTTON_OFFSET_Y;
	GuiButton* userColorButton = createButton(aiWindow->generalProperties.wrapper, btnBounds,
		userColorButtonZOrder, NULL, BROWN, onUserColorClick);
	if ((NULL == userColorButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	btnBounds.y = AI_LEVEL_BUTTON_OFFSET_Y;
	GuiButton* aiLevelButton = createButton(aiWindow->generalProperties.wrapper, btnBounds,
		aiLevelButtonZOrder, NULL, BROWN, onAILevelClick);
	if ((NULL == aiLevelButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	btnBounds.y = START_BUTTON_OFFSET_Y;
	GuiButton* startButton = createButton(aiWindow->generalProperties.wrapper, btnBounds,
		startButtonZOrder, BUTTON_START_IMG, BROWN, onStartClick);
	if ((NULL == startButton) || g_guiError || g_memError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	Rectangle titleUserColorBounds = { ((WIN_W - (TITLE_USER_COLOR_IMG_WIDTH)) / 2),
		USER_COLOR_BUTTON_OFFSET_Y - TITLE_USER_COLOR_GAP_HEIGHT - TITLE_USER_COLOR_IMG_HEIGHT,
		TITLE_USER_COLOR_IMG_WIDTH, TITLE_USER_COLOR_IMG_HEIGHT };
	GuiImage* titleUserColor = createImage(aiWindow->generalProperties.wrapper, titleUserColorBounds,
		userColorTitleZOrder, TITLE_USER_COLOR_IMG, MAGENTA);
	if ((NULL == titleUserColor) || g_guiError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	Rectangle titleAiLevelBounds = { ((WIN_W - (TITLE_AI_LEVEL_IMG_WIDTH)) / 2),
		AI_LEVEL_BUTTON_OFFSET_Y - TITLE_AI_LEVEL_GAP_HEIGHT - TITLE_AI_LEVEL_IMG_HEIGHT,
		TITLE_AI_LEVEL_IMG_WIDTH, TITLE_AI_LEVEL_IMG_HEIGHT };
	GuiImage* titleAiLevel = createImage(aiWindow->generalProperties.wrapper, titleAiLevelBounds,
		aiLevelTitleZOrder, TITLE_AI_LEVEL_IMG, MAGENTA);
	if ((NULL == titleAiLevel) || g_guiError)
	{ // Clean on errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	// Create the window extent object
	AIWindowExtent* windowExtent = createAIWindowExtent(aiWindow, NULL);
	if ((NULL == windowExtent) || g_memError || g_guiError)
	{ // Avoid errors
		destroyAIWindow(aiWindow);
		return NULL;
	}

	aiWindow->generalProperties.extent = windowExtent;

	// The user color and ai level buttons are dynamic, load their initial image value from the shared resources
	// according to the settings
	GuiImage* userColorBGImg = (g_isUserBlack ? windowExtent->blackImg : windowExtent->whiteImg);
	GuiImage* aiLevelBGImg = (g_isDifficultyBest ? windowExtent->bestDepthImg : windowExtent->depthImg[g_minimaxDepth - 1]);
	userColorButton->setBGImage(userColorButton, userColorBGImg);
	aiLevelButton->setBGImage(aiLevelButton, aiLevelBGImg);

	// Save a reference to the ai window in the button extents.
	// This makes the game control and other useful info available on events (via the window extent).
	userColorButton->generalProperties.extent = aiWindow;
	aiLevelButton->generalProperties.extent = aiWindow;
	startButton->generalProperties.extent = aiWindow;

	return aiWindow;
}