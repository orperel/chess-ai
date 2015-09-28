#ifndef GUI_FW_
#define GUI_FW_

#include "Types.h"
#include <SDL.h>
#include <SDL_video.h>

//  --------------------------- 
//  -- Type definitions      --
//  ---------------------------

/** A helper red-blue-green color structure */
struct GuiColorRGB
{
	int r, g, b;
};
typedef struct GuiColorRGB GuiColorRGB;

/** Enum for differentiating between the component types since c does not support type detection during runtime. */
typedef enum
{
	WINDOW,
	PANEL,
	BUTTON,
	IMAGE,
	ANIMATION
} GuiComponentType;

/** A general wrapper for all GUI components, to allow storing them in a unified LinkedList.
 *  We use this struct as a basis for "pseudo polymorphism" implementation of functions (e.g: general draw, destroy).
 */
typedef struct
{
	GuiComponentType type;
	void* component;
} GuiComponentWrapper;

/** A rectangle, as used by the applciation's gui FW. */
struct Rectangle
{
	int x, y, width, height;
};
typedef struct Rectangle Rectangle;

typedef struct GuiWindow GuiWindow; // Forward decleration for GuiWindow

/** "Abstract Properties" common to all gui components */
struct GuiGeneralProperties
{
	Rectangle bounds; // Bounds of the control, in local coordinates (relative to the parent container)
	Rectangle visibleBounds; // Visible Bounds of the control, in absolute coordinates (relative to the window (0, 0))
	unsigned short zOrder; // Defines which component gets drawn first (lower Z order means draw at the back)
	GuiComponentWrapper* wrapper; // Wrapper of the current control
	GuiComponentWrapper* parent; // Wrapper of the parent control containing the current control
	GuiWindow* window; // The window that contains the control

	void(*draw)(void* component, const Rectangle* const container); // Draws the component
	void(*destroy)(void* component); // Destroyes the component

};
typedef struct GuiGeneralProperties GuiGeneralProperties;

/** A window in the Gui FW. All components reside inside a window and a window has no parent.
 *	The window is backed by a SDL surface created with it.
 */
struct GuiWindow
{
	GuiGeneralProperties generalProperties;
	GuiColorRGB bgColor; // Background color of window
	const char* title;   // Null terminated string, preallocated and defines the title shown on the window's top
	LinkedList* subComponents; // List of GuiComponentWrapper children, wrapping the children controls

	SDL_Surface* surface; // The SDL surface that this window represents

	void(*show)(struct GuiWindow* window); // Draws the windows and all inner components in the UI tree

};

/** A panel in the Gui FW is a simple container for other components.
 *	It may contain a background color (a simple colored rect behind all controls).
 *	All controls inside the panel are drawn relative to the panel (0, 0) coordinates.
 */
struct GuiPanel
{
	GuiGeneralProperties generalProperties;
	GuiColorRGB bgColor;
	LinkedList* subComponents;

};
typedef struct GuiPanel GuiPanel;

/** A single image texture object in the Gui FW.
 *	An image may represent more than one picture, so we use the scissor region to define which part of the
 *	image we want to draw. That way image may be a stand alone control, or define part of the logic of other controls
 *	e.g: (button and animation).
 */
struct GuiImage
{
	GuiGeneralProperties generalProperties;
	GuiColorRGB transparentColor; // The color that represents the transparent color in the image
	const char* sourcePath; // Source to load the image bitmap from
	Rectangle scissorRegion; // Which part of the image texture should actually be drawn
	SDL_Surface* surface; // The SDL surface that contains the bitmap texture

};
typedef struct GuiImage GuiImage;

/** State of the button, depending on user events (mouse is over the button, user clicked it, etc) */
typedef enum
{
	DEFAULT,
	MOUSE_MOVE,
	MOUSE_DOWN,
	MOUSE_UP
} ButtonState;

/** A button in the Gui FW. A button is clickable and may perform a user-defined operation (onClick).
 *  The button is drawn and represented by the bgImage. Buttons images are expected to be in
 *	dimensions multiplied by 2, so they contain the layout for each of the 4 button states.
 */
struct GuiButton
{
	GuiGeneralProperties generalProperties;
	GuiImage* bgImage;

	ButtonState state;

	void(*onClick)(struct GuiButton* button);

};
typedef struct GuiButton GuiButton;

/** An animation in the Gui FW. The animation is backed by a single image texture, containing may clips.
 *	Clips are aligned inside the image, and the image's dimensions must be multiplications of clipWidth and clipHeight.
 *	The rate of the animation is defined by timeBetweenFramesMs.
 *  Animations may be repeatable or non-repeatable (animation stays in the last frame).
 *	They may also be restarted manually (restart()).
 */
struct GuiAnimation
{
	GuiGeneralProperties generalProperties;
	GuiImage* clips;
	int clipWidth, clipHeight;

	int timeBetweenFramesMs;
	bool isRepeated;

	short currentClip;
	int clipChangeStartTime;

	void(*restart)(struct GuiAnimation* animation);
	void(*onAnimationEnd)(struct GuiAnimation* animation);

};
typedef struct GuiAnimation GuiAnimation;


//  --------------------------- 
//  -- Constants and globals --
//  ---------------------------

/** Error code returned when the framework terminates successfully */
#define OK_EXIT_CODE 0

/** Error code returned when SDL fails to initialize */
#define SDL_FAILURE_EXIT_CODE 1

/** Error code returned when a gui error occures */
#define GUI_ERROR_EXIT_CODE 2

/* True if there was a gui error somewhere in the program (would cause an exit). Else false. */
extern bool g_guiError;

/** Definitions of common colors in RGB */
extern const GuiColorRGB RED;
extern const GuiColorRGB GREEN;
extern const GuiColorRGB BLUE;
extern const GuiColorRGB WHITE;
extern const GuiColorRGB BLACK;
extern const GuiColorRGB MAGENTA;
extern const GuiColorRGB YELLOW;
extern const GuiColorRGB AQUA;
extern const GuiColorRGB PURPLE;
extern const GuiColorRGB ORANGE;
extern const GuiColorRGB PINK;
extern const GuiColorRGB GRAY;
extern const GuiColorRGB BROWN;

/** Framerate of the rendering system. */
#define FRAME_RATE 60
extern const unsigned int TIME_BETWEEN_FRAMES_MS;

//  --------------------------- 
//  -- API functions         --
//  ---------------------------

/** Initializes the SDL part of the Gui FW, and perpares the system for creation of Gui components. */
int initGui();

/** Creates a blank new window in the Gui FW. On error, NULL is returned.
 *  Width, height - dimensions of the window.
 *	Title - the title at the top of the window.
 *	bgColor - the background color of the window
 */
GuiWindow* createWindow(int width, int height, const char* title, GuiColorRGB bgColor);

/** Creates a blank new panel in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *	bgColor - the background color of the control.
 */
GuiPanel* createPanel(GuiComponentWrapper* parent, Rectangle bounds, short zOrder, GuiColorRGB bgColor);

/** Creates a new image in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *  sourcePath - relative path of the bitmap to be drawn inside the image.
 *	transparentColor - The color that represents transparency in the bitmap.
 */
GuiImage* createImage(GuiComponentWrapper* parent, Rectangle bounds, short zOrder,
					  const char* sourcePath, GuiColorRGB transparentColor);

/** Creates a new animation in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *  sourcePath - relative path of the bitmap to be drawn inside the image (all animation clips are contained within this image).
 *	transparentColor - The color that represents transparency in the bitmap.
 *  clipWidth, clipHeight - Scissored region of each single clip (frame) within the bigger animation image.
 *	timeBetweenFrames - Time in milliseconds between frames in the animation. The system may not maintain an exact
 *						framerate but will try to stay as close to it as possible.
 *	isRepeated - True - the animation starts over when finished. False - the animation remains on the last frame when finished.
 *	onAnimationEnd - an event prompted when the animation ends.
 */
GuiAnimation* createAnimation(GuiComponentWrapper* parent, Rectangle bounds, short zOrder,
						  	  const char* sourcePath, GuiColorRGB transparentColor,
							  int clipWidth, int clipHeight,
							  int timeBetweenFramesMs, bool isRepeated,
							  void(*onAnimationEnd)(GuiAnimation* animation));

/** Creates a new animation in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *  imageSourcePath - relative path of the bitmap to be drawn inside the button (all 4 button states).
 *	transparentColor - The color that represents transparency in the bitmap.
 *	onClick - an event prompted when the button is clicked.
 */
GuiButton* createButton(GuiComponentWrapper* parent, Rectangle bounds, short zOrder,
						const char* imageSourcePath, GuiColorRGB transparentColor,
						void(*onClick)(GuiButton* button));

/** Destructor for Gui windows.
 *	Destroys all inner components as well.
 */
void destroyWindow(void* component);

/** Make the window visible. */
void showWindow(GuiWindow* window);

/** Restarts an animation to the first clip. */
void restartAnimation(GuiAnimation* animation);

/** Queries which SDL events have happened and prompts corresponding events in the Gui FW.
 *  (e.g: GuiButton was clicked).
 */
bool processGuiEvents(GuiWindow* activeWindow);

#endif