#include "GuiFW.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//  ---------------------------------------
//  -- Constants and globals definitions --
//  ---------------------------------------

/* True if there was a gui error somewhere in the program (would cause an exit). Else false. */
bool g_guiError = false;

/** Definitions of common colors in RGB. To be used by users and the framework alike. */
const GuiColorRGB RED = { 255, 0, 0 };
const GuiColorRGB GREEN = { 0, 255, 0 };
const GuiColorRGB BLUE = { 0, 0, 255 };
const GuiColorRGB WHITE = { 255, 255, 255 };
const GuiColorRGB BLACK = { 0, 0, 0 };
const GuiColorRGB MAGENTA = { 255, 0, 255 };
const GuiColorRGB YELLOW = { 255, 255, 0 };
const GuiColorRGB AQUA = { 0, 255, 255 };
const GuiColorRGB PURPLE = { 128, 0, 255 };
const GuiColorRGB ORANGE = { 255, 128, 0 };
const GuiColorRGB PINK = { 255, 128, 255 };
const GuiColorRGB GRAY = { 128, 128, 128 };
const GuiColorRGB BROWN = { 128, 0, 0 };

// Time between frames, depends on the framerate (in ms)
const unsigned int TIME_BETWEEN_FRAMES_MS = (1000 / FRAME_RATE);

// Dimensions for spacing dialog buttons in their container
const unsigned int DIALOG_BUTTON_OFFSET_X = 20; // Initial X for all buttons
const unsigned int DIALOG_BUTTON_OFFSET_Y = 20; // Initial Y for first button
const unsigned int DIALOG_BUTTON_OFFSET_GAP = 7; // Gap in Y axis between dialog buttons
const unsigned int DIALOG_FRAME_MARGIN = 2;

// The extent additional info attached to each button in gui dialogs.
// This is used for proper processing of dialog choices (this extent should be private to outside users).
typedef struct GuiDialogButtonExtent
{
	GuiDialog* dialog; // The containing dialog
	void* choiceData; // Data attached to the dialog button
} GuiDialogButtonExtent;

//  ------------------------------ 
//  -- Forward declerations     --
//  ------------------------------

void drawWindow(void* component, const Rectangle* const container);
void drawPanel(void* component, const Rectangle* const container);
void drawButton(void* component, const Rectangle* const container);
void drawImage(void* component, const Rectangle* const container);
void drawAnimation(void* component, const Rectangle* const container);
void drawDialog(void* component, const Rectangle* const container);
void destroyGuiComponentWrapper(void* wrapper);
void destroyWindow(void* component);
void destroyButton(void* component);
void destroyImage(void* component);
void destroyPanel(void* component);
void destroyAnimation(void* component);
void destroyDialog(void* component);
void destroyDialogButton(void* component);
void removeComponentFromParent(GuiComponentWrapper* wrapper);
bool processGuiEventsForBranch(GuiComponentWrapper* wrapper);


//  ------------------------------ 
//  -- General helper functions --
//  ------------------------------

/** Returns the general properties category of a gui component.
 *	The wrapper determines the component type and allows a "pseudo-polymorphism" implementation.
 */
GuiGeneralProperties* getComponentGeneralProperties(GuiComponentWrapper* componentWrapper)
{
	GuiGeneralProperties* properties = NULL;

	// We wrap the components to enable "pseudo-polymorphic" access to them
	switch (componentWrapper->type)
	{
		case(WINDOW) :
		{
			GuiWindow* window = (GuiWindow*)componentWrapper->component;
			properties = &window->generalProperties;
			break;
		}
		case(PANEL) :
		{
			GuiPanel* panel = (GuiPanel*)componentWrapper->component;
			properties = &panel->generalProperties;
			break;
		}
		case(BUTTON) :
		case(DIALOG_BUTTON) :
		{
			GuiButton* button = (GuiButton*)componentWrapper->component;
			properties = &button->generalProperties;
			break;
		}
		case(IMAGE) :
		{
			GuiImage* image = (GuiImage*)componentWrapper->component;
			properties = &image->generalProperties;
			break;
		}
		case(ANIMATION) :
		{
			GuiAnimation* animation = (GuiAnimation*)componentWrapper->component;
			properties = &animation->generalProperties;
			break;
		}
		case(DIALOG) :
		{
			GuiDialog* dialog = (GuiDialog*)componentWrapper->component;
			properties = &dialog->generalProperties;
			break;
		}
		default:
		{
			printf("ERROR: Undefined component type in getComponentGeneralProperties.\n");
			g_guiError = true;
			properties = NULL;
		}
	}

	return properties;
}

/** Adsd the child component to the list of components, and maintains the lists ascending z-order. */
void addChildToSortedZOrderList(LinkedList* list, GuiComponentWrapper* newItem)
{
	GuiGeneralProperties* newProperties = getComponentGeneralProperties(newItem);
	unsigned short newZOrder = newProperties->zOrder;

	Node* currNode = list->head;
	Node* prevNode = NULL;

	// Advance along the list until the sorted Z order position for the item is found, or the end is encountered
	while ((NULL != currNode) && (newZOrder >= getComponentGeneralProperties(currNode->data)->zOrder))
	{
		prevNode = currNode;
		currNode = currNode->next;
	}

	// All cases should be taken care of by the linked-list implementation:
	// Case #1: List is empty -- prevNode is NULL and new node is inserted in the beginning.
	// Case #2: Node inserted before node with higher z-order -- prevNode points the node before that node
	//			so insert between them (or beginning of the list if currNode is the first node).
	// Case #3: Node inserted after a node with similar z-order -- should be treated as case #2.
	// Case #4: New node is with highest z-order so far -- node is inserted after the last node.
	insertAfter(list, prevNode, newItem);
}

/** Creates a wrapper that represents the control in the UI tree. The control and wrapper point to each other.
 *  The componentType determines the type of wrapper that is created and must match the component parameter.
 */
GuiComponentWrapper* createControlWrapper(void* component, GuiComponentType componentType)
{
	GuiComponentWrapper* wrapper = (GuiComponentWrapper*)malloc(sizeof(GuiComponentWrapper));
	if (NULL == wrapper)
	{
		printf("Error: standard function malloc has failed");
		g_guiError = true;
		return NULL;
	}

	wrapper->type = componentType;
	wrapper->component = component;

	return wrapper;
}

/** Attaches the child (wrapper) to the parent (links the child component to the correct field of the parent component).
 */
void addChildComponent(GuiComponentWrapper* wrapper, GuiComponentWrapper* parent)
{
	// We wrap the components to enable "pseudo-polymorphic" access to them
	switch (parent->type)
	{
		case(WINDOW) :
		{
			GuiWindow* window = (GuiWindow*)parent->component;
			addChildToSortedZOrderList(window->subComponents, wrapper);
			break;
		}
		case(PANEL) :
		{
			GuiPanel* panel = (GuiPanel*)parent->component;
			addChildToSortedZOrderList(panel->subComponents, wrapper);
			break;
		}
		case(BUTTON) :
		case(DIALOG_BUTTON) :
		{
			GuiButton* button = (GuiButton*)parent->component;

			if (IMAGE != wrapper->type)
			{
				printf("Error: Invalid state. Button can only have image son components");
				g_guiError = true;
				break;
			}

			button->bgImage = (GuiImage*)wrapper->component; // Attach son to specific field
			break;
		}
		case(IMAGE) :
		{
			printf("Error: Invalid state. Image components are always leaves in the UI tree");
			g_guiError = true;
			break;
		}
		case(ANIMATION) :
		{
			GuiAnimation* animation = (GuiAnimation*)parent->component;

			if (IMAGE != wrapper->type)
			{
				printf("Error: Invalid state. Animation can only have image son components");
				g_guiError = true;
				break;
			}

			animation->clips = (GuiImage*)wrapper->component; // Attach son to specific field
			break;
		}
		case(DIALOG) :
		{
			GuiDialog* dialog = (GuiDialog*)parent->component;

			// A dialog has one panel (under that panel there is one background image and a list of buttons).
			if (PANEL == wrapper->type)
			{
				dialog->dialogPanel = (GuiPanel*)wrapper->component; // Attach son to specific field
			}
			else
			{
				printf("Error: Invalid state. Dialog can only have panel son components");
				g_guiError = true;
				break;
			}

			break;
		}
		default:
		{
			printf("ERROR: Undefined parent component type in addChildComponent.\n");
			g_guiError = true;
			break;
		}
	}
}

/** Make the window visible. */
void showWindow(GuiWindow* window)
{
	drawWindow(window, &window->generalProperties.bounds);
}

void setBGImage(GuiButton* button, GuiImage* image)
{
	// If the button owned a previous bg image, dispose it
	if ((NULL != button->bgImage) && (button->isSurfaceOwner))
	{
		destroyImage(button->bgImage);
		button->bgImage = NULL;
	}

	// Attach image and button
	addChildComponent(image->generalProperties.wrapper, button->generalProperties.wrapper);
	image->generalProperties.parent = button->generalProperties.wrapper;

	// Mark the external image memory is not managed by the button
	button->isSurfaceOwner = false;
}

//  ------------------------------ 
//  -- Create functions         --
//  ------------------------------

/** Creates a blank new window in the Gui FW. On error, NULL is returned.
 *  Width, height - dimensions of the window.
 *	Title - the title at the top of the window.
 *	bgColor - the background color of the window
 */
GuiWindow* createWindow(int width, int height, const char* title, GuiColorRGB bgColor)
{
	GuiWindow* window = (GuiWindow*)malloc(sizeof(GuiWindow));
	if (NULL == window)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	Rectangle defaultBounds = { 0, 0, width, height };
	window->generalProperties.bounds = defaultBounds;
	window->generalProperties.zOrder = 0; // Window is always at the very back
	window->generalProperties.parent = NULL; // Windows are always at the root
	window->generalProperties.window = window; // The containing window is the window itself
	window->generalProperties.isVisible = true;
	window->generalProperties.visibleBounds = defaultBounds; // Window is always in absolute coords
	window->generalProperties.extent = NULL;
	window->bgColor = bgColor;
	window->title = title;
	window->isWindowQuit = false;
	window->subComponents = createList(destroyGuiComponentWrapper);

	// Set "control methods"
	window->generalProperties.draw = drawWindow;
	window->generalProperties.destroy = destroyWindow;
	window->generalProperties.wrapper = createControlWrapper(window, WINDOW);
	if (NULL == window->generalProperties.wrapper)
	{
		return NULL;
	}

	window->show = showWindow;

	// SDL initialization
	SDL_WM_SetCaption(window->title, window->title); // Set window caption
	window->surface = SDL_SetVideoMode(width, height, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);

	if (NULL == window->surface)
	{
		g_guiError = true;
		printf("ERROR: unable to create SDL surface: %s\n", SDL_GetError());
		return NULL;
	}

	return window;
}

/** Creates a blank new panel in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control. 
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *	bgColor - the background color of the control.
 */
GuiPanel* createPanel(GuiComponentWrapper* parent, Rectangle bounds, short zOrder, GuiColorRGB bgColor)
{
	if (NULL == parent)
	{
		printf("Error: NULL parent given to new panel component");
		return NULL;
	}

	GuiPanel* panel = (GuiPanel*)malloc(sizeof(GuiPanel));
	if (NULL == panel)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	panel->generalProperties.bounds = bounds;
	panel->generalProperties.parent = parent;
	panel->generalProperties.window = getComponentGeneralProperties(parent)->window;
	panel->generalProperties.zOrder = zOrder;
	panel->generalProperties.isVisible = true;
	Rectangle emptyBounds = { 0, 0, 0, 0 };
	panel->generalProperties.visibleBounds = emptyBounds; // Will be calculated on first draw
	panel->generalProperties.extent = NULL;
	panel->bgColor = bgColor;
	panel->subComponents = createList(destroyGuiComponentWrapper);
	panel ->generalProperties.wrapper = createControlWrapper(panel, PANEL);
	if (NULL == panel->generalProperties.wrapper)
	{
		return NULL;
	}

	// Set "control methods"
	panel->generalProperties.draw = drawPanel;
	panel->generalProperties.destroy = destroyPanel;

	addChildComponent(panel->generalProperties.wrapper, parent);

	return panel;
}

/** Creates a new image in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *  sourcePath - relative path of the bitmap to be drawn inside the image.
 *	transparentColor - The color that represents transparency in the bitmap.
 */
GuiImage* createImage(GuiComponentWrapper* parent, Rectangle bounds, short zOrder, const char* sourcePath, GuiColorRGB transparentColor)
{
	if (NULL == parent)
	{
		printf("Error: NULL parent given to new image component");
		return NULL;
	}

	GuiImage* image = (GuiImage*)malloc(sizeof(GuiImage));
	if (NULL == image)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	image->generalProperties.bounds = bounds;
	image->generalProperties.parent = parent;
	image->generalProperties.zOrder = zOrder;
	image->generalProperties.window = getComponentGeneralProperties(parent)->window;
	image->generalProperties.isVisible = true;
	image->sourcePath = sourcePath;
	image->transparentColor = transparentColor;
	image->scissorRegion.x = 0; // By default draw the entire image texture
	image->scissorRegion.y = 0;
	image->scissorRegion.width = bounds.width;
	image->scissorRegion.height = bounds.height;
	Rectangle emptyBounds = { 0, 0, 0, 0 };
	image->generalProperties.visibleBounds = emptyBounds; // Will be calculated on first draw
	image->generalProperties.extent = NULL;
	image->generalProperties.wrapper = createControlWrapper(image, IMAGE);
	if (NULL == image->generalProperties.wrapper)
	{
		return NULL;
	}

	// Set "control methods"
	image->generalProperties.draw = drawImage;
	image->generalProperties.destroy = destroyImage;

	addChildComponent(image->generalProperties.wrapper, parent);

	// SDL initialization
	image->surface = SDL_LoadBMP(sourcePath);

	if (NULL == image->surface)
	{
		g_guiError = true;
		printf("ERROR: unable to load bitmap \"%s\" to SDL surface: %s\n", sourcePath, SDL_GetError());
		return NULL;
	}

	return image;
}

/** Restarts an animation to the first clip. */
void restartAnimation(GuiAnimation* animation)
{
	animation->currentClip = 0;
	animation->clipChangeStartTime = 0;
}

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
							  void(*onAnimationEnd)(GuiAnimation* animation))
{
	if (NULL == parent)
	{
		printf("Error: NULL parent given to new animation component");
		return NULL;
	}

	GuiAnimation* animation = (GuiAnimation*)malloc(sizeof(GuiAnimation));
	if (NULL == animation)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	animation->generalProperties.bounds = bounds;
	animation->generalProperties.parent = parent;
	animation->generalProperties.zOrder = zOrder;
	animation->generalProperties.window = getComponentGeneralProperties(parent)->window;
	animation->generalProperties.isVisible = true;
	Rectangle emptyBounds = { 0, 0, 0, 0 };
	animation->generalProperties.visibleBounds = emptyBounds; // Will be calculated on first draw
	animation->generalProperties.extent = NULL;
	animation->clipWidth = clipWidth;
	animation->clipHeight = clipHeight;
	animation->timeBetweenFramesMs = timeBetweenFramesMs;
	animation->clipChangeStartTime = 0;
	animation->isRepeated = isRepeated;
	animation->currentClip = 0;
	Rectangle imageBounds = { bounds.x, bounds.y, bounds.width, bounds.height };
	animation->generalProperties.wrapper = createControlWrapper(animation, ANIMATION);
	if (NULL == animation->generalProperties.wrapper)
	{
		return NULL;
	}

	// Create image son component. It will link itself to the animation parent.
	createImage(animation->generalProperties.wrapper, imageBounds, zOrder, sourcePath, transparentColor);

	// Set "control methods"
	animation->generalProperties.draw = drawAnimation;
	animation->generalProperties.destroy = destroyAnimation;
	animation->restart = restartAnimation;

	// Set "control events"
	animation->onAnimationEnd = onAnimationEnd;

	// Attach the animation to its parent.
	addChildComponent(animation->generalProperties.wrapper, parent);

	return animation;
}

/** Creates a new button in the Gui FW. On error, NULL is returned.
 *  parent - Control that contains this new control.
 *  bounds - position and dimensions of the control.
 *	zOrder - Sorts which control shows in front of which, the higher z order the closer the control is to the user.
 *  imageSourcePath - relative path of the bitmap to be drawn inside the button (all 4 button states).
 *	transparentColor - The color that represents transparency in the bitmap.
 *	onClick - an event prompted when the button is clicked.
 */
GuiButton* createButton(GuiComponentWrapper* parent, Rectangle bounds, short zOrder,
						const char* imageSourcePath, GuiColorRGB transparentColor,
						void(*onClick)(GuiButton* button))
{
	if (NULL == parent)
	{
		printf("Error: NULL parent given to new button component");
		return NULL;
	}

	GuiButton* button = (GuiButton*)malloc(sizeof(GuiButton));
	if (NULL == button)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	button->generalProperties.bounds = bounds;
	button->generalProperties.parent = parent;
	button->generalProperties.zOrder = zOrder;
	button->generalProperties.window = getComponentGeneralProperties(parent)->window;
	button->generalProperties.isVisible = true;
	Rectangle emptyBounds = { 0, 0, 0, 0 };
	button->generalProperties.visibleBounds = emptyBounds; // Will be calculated on first draw
	button->generalProperties.extent = NULL;
	button->generalProperties.wrapper = createControlWrapper(button, BUTTON);
	if (NULL == button->generalProperties.wrapper)
	{
		return NULL;
	}

	// Default button state
	button->state = DEFAULT;
	button->isEnabled = true; // By default buttons are enabled
	button->isSurfaceOwner = false;

	// Create image only if bgImage path was supplied
	if (NULL != imageSourcePath)
	{
		// Create image son component. It will link itself to the button parent.
		Rectangle imageBounds = { bounds.x, bounds.y, bounds.width, bounds.height };
		createImage(button->generalProperties.wrapper, imageBounds, zOrder, imageSourcePath, transparentColor);
		button->isSurfaceOwner = true; // If the background image is specified than the button manages the bg image object
	}
	else
	{
		button->bgImage = NULL; // Explicitly set to null, until user sets a bg image
	}

	// Set "control methods"
	button->generalProperties.draw = drawButton;
	button->generalProperties.destroy = destroyButton;
	button->setBGImage = setBGImage;

	// Set "control events"
	button->onClick = onClick;

	// Attach the button to its parent.
	addChildComponent(button->generalProperties.wrapper, parent);

	return button;
}

/** Delegates the button click event to a dialog button click event, for dialogs. */
void onDialogButtonClick(GuiButton* button)
{
	GuiDialogButtonExtent* extent = (GuiDialogButtonExtent*)button->generalProperties.extent;
	extent->dialog->choice = extent->choiceData; // Signals the dialog can be closed, now that the choice is made
}

/** Adds an option to the dialog. The option will be added as a button with the image as background
 *  (transparency belongs to the image of the button's background). choiceData is the results returned if
 *	the button option is picked. Options appear in the order they are added in.
 */
void addDialogOption(GuiDialog* dialog, const char* imageSourcePath, GuiColorRGB transparentColor, void* choiceData)
{
	int singleButtonHeight = (dialog->choiceButtonHeight / 2); // Buttons contain animation of 4 states so we halve by 2
	int singleButtonWidth = (dialog->choiceButtonWidth / 2);

	// Every time we cross choicesPerColumn, a new column of buttons is added to the dialog,
	// so we place the following buttons more to the right
	int offsetX = DIALOG_BUTTON_OFFSET_X +
		((dialog->numOfChoices) / dialog->choicesPerColumn) * (singleButtonWidth + DIALOG_BUTTON_OFFSET_GAP);

	// Depends on number of choices, but if we are in a new column, place it according to the beginning of the column.
	// Thats why we use modulo here.
	int offsetY = DIALOG_BUTTON_OFFSET_Y +
		(dialog->numOfChoices % dialog->choicesPerColumn) * (singleButtonHeight + DIALOG_BUTTON_OFFSET_GAP);
	Rectangle buttonBounds = { offsetX, offsetY, dialog->choiceButtonWidth, dialog->choiceButtonHeight };
	GuiButton* dialogButton = 
		createButton(dialog->dialogPanel->generalProperties.wrapper, buttonBounds, 2, imageSourcePath,
					 transparentColor, onDialogButtonClick);

	if (NULL == dialogButton)
	{
		g_guiError = true;
		return;
	}

	// Override button type to dialog - this should ensure the right destructor is called when the button is deallocated
	// Also set the correct destructor
	dialogButton->generalProperties.wrapper->type = DIALOG_BUTTON;
	dialogButton->generalProperties.destroy = destroyDialogButton;

	GuiDialogButtonExtent* extent = (GuiDialogButtonExtent*)malloc(sizeof(GuiDialogButtonExtent));
	if (NULL == extent)
	{
		g_guiError = true;
		printf("Error: standard function malloc has failed");
		return;
	}

	// Store data in extent for future events
	extent->dialog = dialog;
	extent->choiceData = choiceData;
	dialogButton->generalProperties.extent = extent;

	if (dialog->numOfChoices < dialog->choicesPerColumn)
	{ // Dialog should increase its height until we fill the first column
		int heightAddition = singleButtonHeight + DIALOG_BUTTON_OFFSET_GAP;
		dialog->generalProperties.bounds.height += heightAddition;
		dialog->generalProperties.bounds.y -= (DIALOG_BUTTON_OFFSET_GAP + singleButtonHeight) / 2;
		dialog->dialogPanel->generalProperties.bounds = dialog->generalProperties.bounds;
		dialog->bgImage->generalProperties.bounds.height += heightAddition;
		dialog->bgImage->scissorRegion.height += heightAddition;
	}
	else if ((dialog->numOfChoices > 1) && (dialog->numOfChoices % dialog->choicesPerColumn == 1))
	{ // Dialog should increase its width if we've hit the max per column
		int widthAddition = singleButtonWidth + DIALOG_BUTTON_OFFSET_GAP;
		dialog->generalProperties.bounds.width += widthAddition;
		dialog->generalProperties.bounds.x -= (DIALOG_BUTTON_OFFSET_GAP + singleButtonWidth) / 2;
		dialog->dialogPanel->generalProperties.bounds = dialog->generalProperties.bounds;
		dialog->bgImage->generalProperties.bounds.width += widthAddition;
		dialog->bgImage->scissorRegion.width += widthAddition;
	}

	dialog->numOfChoices++;
}

/** Shows the dialog in a modal way. The app is stuck on the dialog until a choice is made
 *	(to give a "modal window" effect).
 */
void* showDialog(GuiDialog* dialog)
{
	GuiWindow* window = dialog->generalProperties.window;
	int lastRenderTime = SDL_GetTicks();
	showWindow(window); // Redraw the window with the dialog

	// Loop until a choice is made by the user.
	// This gives the dialog an attribute of a "modal window" (the app is "stuck" until a choice is made).
	// We make sure to process events here so the GUI feels natural.
	while (!window->isWindowQuit &&
		   !processGuiEventsForBranch(dialog->generalProperties.wrapper) &&
		   (NULL == dialog->choice))
	{
		// We process events until one of the dialog buttons is clicked and the dialog is filled with a choice.
		// We keep redrawing to support onMouseOver and onMouseDown animations

		if (SDL_GetTicks() - lastRenderTime > TIME_BETWEEN_FRAMES_MS)
		{
			showWindow(window);
			lastRenderTime = SDL_GetTicks();
		}

		SDL_Delay(TIME_BETWEEN_FRAMES_MS); // Maintain a framerate as well.
	}

	void* result = dialog->choice; // Fetch the choice the user have made
	removeComponentFromParent(dialog->generalProperties.wrapper); // Destroy the dialog component and its wrapper

	return result;
}

/** Creates a dialog inner window in the Gui FW. On error, NULL is returned.
 *	Note: A dialog is created with empty options, they should be added manually.
 *  parent - Window that contains this new control (dialog's parent must always be a window).
 *  bgImageSourcePath - relative path of the bitmap of the background image.
 *	bgImgtransparentColor - The color that represents transparency in the background bitmap.
 *	defaultBGColor - Color of the panel behind the bg image (for cases where the image doesn't span on the entire dialog,
 *					 or transparency occurs).
 */
GuiDialog* createDialog(GuiWindow* parent, int choiceButtonWidth, int choiceButtonHeight,
						const char* bgImageSourcePath, GuiColorRGB bgImgTransparentColor,
						GuiColorRGB defaultBGColor)
{
	if (NULL == parent)
	{
		printf("Error: NULL parent given to new dialog component");
		return NULL;
	}

	GuiDialog* dialog = (GuiDialog*)malloc(sizeof(GuiDialog));
	if (NULL == dialog)
	{
		printf("Error: standard function malloc has failed");
		return NULL;
	}

	// Set "control properties"
	Rectangle bounds; // These are the basic bounds according to we place the inner components.
	bounds.width = (choiceButtonWidth / 2) + (DIALOG_BUTTON_OFFSET_X * 2);
	bounds.height = (DIALOG_BUTTON_OFFSET_Y * 2);
	bounds.x = (parent->generalProperties.bounds.width / 2) - (bounds.width / 2); // Center the dialog
	bounds.y = (parent->generalProperties.bounds.height / 2) - (bounds.height / 2);

	Rectangle dialogBounds = bounds;
	dialogBounds.x -= DIALOG_FRAME_MARGIN;
	dialogBounds.y -= DIALOG_FRAME_MARGIN;
	dialogBounds.width += DIALOG_FRAME_MARGIN * 2;
	dialogBounds.height += DIALOG_FRAME_MARGIN * 2;
	dialog->generalProperties.bounds = dialogBounds;
	dialog->generalProperties.parent = parent->generalProperties.wrapper;
	dialog->generalProperties.zOrder = SHRT_MAX; // Dialogs are displayed in front of everything
	dialog->generalProperties.window = getComponentGeneralProperties(parent->generalProperties.wrapper)->window;
	dialog->generalProperties.isVisible = true;
	Rectangle emptyBounds = { 0, 0, 0, 0 };
	dialog->generalProperties.visibleBounds = emptyBounds; // Will be calculated on first draw
	dialog->generalProperties.extent = NULL;
	dialog->generalProperties.wrapper = createControlWrapper(dialog, DIALOG);
	if (NULL == dialog->generalProperties.wrapper)
	{
		g_guiError = true;
		return NULL;
	}

	dialog->choiceButtonWidth = choiceButtonWidth;
	dialog->choiceButtonHeight = choiceButtonHeight;
	dialog->numOfChoices = 0;
	dialog->choice = NULL; // Signals that no choice have been made in the dialog yet

	// By default use the constant number. Users can override this setting
	dialog->choicesPerColumn = MAX_OPTIONS_PER_DIALOG_COL;

	// Create a panel at the very back
	createPanel(dialog->generalProperties.wrapper, dialogBounds, 0, defaultBGColor);

	// Create image background component. It will link itself to the panel parent.
	bounds.x = DIALOG_FRAME_MARGIN;
	bounds.y = DIALOG_FRAME_MARGIN;
	dialog->bgImage = 
		createImage(dialog->dialogPanel->generalProperties.wrapper, bounds, 1, bgImageSourcePath, bgImgTransparentColor);

	// Set "control methods"
	dialog->generalProperties.draw = drawDialog;
	dialog->generalProperties.destroy = destroyDialog;
	dialog->addOption = addDialogOption;
	dialog->showDialog = showDialog;

	// Attach the button to its parent.
	addChildComponent(dialog->generalProperties.wrapper, parent->generalProperties.wrapper);

	return dialog;
}

//  ------------------------------ 
//  -- Destroy functions        --
//  ------------------------------

/** Destructor for GuiWrappers. Destroyes the inner component as well and all children under it. */
void destroyGuiComponentWrapper(void* wrapper)
{
	if (NULL == wrapper)
		return;

	if (NULL != ((GuiComponentWrapper*)wrapper)->component)
	{
		// Find the destructor of the inner component and call it.
		// We wrap the components to enable "pseudo-polymorphic" access to them.
		GuiGeneralProperties* properties = getComponentGeneralProperties(wrapper);

		if (NULL != properties)
		{
			properties->destroy(((GuiComponentWrapper*)wrapper)->component);
		}
	}

	free(wrapper); // Finally free the wrapper
}

/** Destructor for Gui windows. */
void destroyWindow(void* component)
{
	if (NULL == component)
		return;

	GuiWindow* window = (GuiWindow*)component;

	// Delete all son components first
	if (NULL != window->subComponents)
		deleteList(window->subComponents);

	if (NULL != window->surface)
	{ // Free SDL resources
		SDL_FreeSurface(window->surface);
	}

	if (NULL != window->generalProperties.wrapper)
	{
		window->generalProperties.wrapper->component = NULL; // Avoid recursive destruction
		destroyGuiComponentWrapper((void*)(window->generalProperties.wrapper));
	}

	free(window); // Finally free allocate window struct
}

/** Destructor for Gui panels. */
void destroyPanel(void* component)
{
	if (NULL == component)
		return;

	GuiPanel* panel = (GuiPanel*)component;

	// Delete all son components first
	if (NULL != panel->subComponents)
		deleteList(panel->subComponents);

	free(panel);
}

/** Destructor for Gui images. */
void destroyImage(void* component)
{
	if (NULL == component)
		return;

	GuiImage* image = (GuiImage*)component;

	if (NULL != image->surface)
	{
		SDL_FreeSurface(image->surface);
	}

	free(image);
}

/** Destructor for Gui buttons. */
void destroyButton(void* component)
{
	if (NULL == component)
		return;

	GuiButton* button = (GuiButton*)component;

	// Release image only if button owns it
	if (button->isSurfaceOwner)
	{
		destroyImage(button->bgImage);
	}

	free(button);
}

/** Destructor for Gui animations. */
void destroyAnimation(void* component)
{
	if (NULL == component)
		return;

	GuiAnimation* animation = (GuiAnimation*)component;

	destroyImage(animation->clips);
	free(animation);
}

/** Destructor for Gui dialogs. */
void destroyDialogButton(void* component)
{
	if (NULL == component)
		return;

	GuiButton* dialogButton = (GuiButton*)component;

	// Dialog buttons have to free their extent before their destruction
	free(dialogButton->generalProperties.extent);
	destroyButton(dialogButton); // Destroy the rest as a normal button
}

/** Removes the component from its parent container (window or panel).
 *  This causes the destruction of the wrapper and its attached component.
 */
void removeComponentFromParent(GuiComponentWrapper* wrapper)
{
	GuiGeneralProperties* generalProperties = getComponentGeneralProperties(wrapper);

	// Look for the component under its parent window and deattach it
	GuiComponentWrapper* parentWrapper = generalProperties->parent;

	// Only panel and window contain a list of child components
	LinkedList* subComponents;

	if (parentWrapper->type == PANEL)
		subComponents = ((GuiPanel*)parentWrapper->component)->subComponents;
	else if (parentWrapper->type == WINDOW)
		subComponents = ((GuiWindow*)parentWrapper->component)->subComponents;
	else
		return;

	Node* currNode = subComponents->head;

	while (NULL != currNode)
	{
		if (currNode->data == wrapper)
			break;
		currNode = currNode->next;
	}

	// Remove the dialog node
	if (NULL != currNode)
		deleteNode(subComponents, currNode);

	// Deleting the node will delete the components's wrapper and then the
	// wrapper itself (the list calls the destructor)
}

/** Concrete destructor for Gui dialogs. */
void destroyDialog(void* component)
{
	if (NULL == component)
		return;

	GuiDialog* dialog = (GuiDialog*)component;

	if (NULL != dialog->dialogPanel)
		destroyPanel(dialog->dialogPanel); // Should free all son dialog buttons, bg image, etc

	free(dialog);
}

//  ------------------------------ 
//  -- Draw functions           --
//  ------------------------------

/** Draws each of the children in the subComponents list.
 *	Children are drawn in consideration to the parent container bounds (so they are drawn relative to the parent coordinates).
 */
void drawChildComponents(LinkedList* subComponents, const Rectangle* const parentBounds)
{
	// Draw each of the child components
	Node* currNode = subComponents->head;

	while (NULL != currNode)
	{
		GuiComponentWrapper* componentWrapper = (GuiComponentWrapper*)currNode->data;
		GuiGeneralProperties* properties = getComponentGeneralProperties(componentWrapper);

		if (NULL != properties)
		{
			// We wrap the components to enable "pseudo-polymorphic" access to them
			properties->draw(componentWrapper->component, parentBounds);
		}

		currNode = currNode->next;
	}
}

/** Draws the window and all sub components inside it.
 *  For windows - the container parameter can be ignored (exists due to polymorphism)
 */
void drawWindow(void* component, const Rectangle* const container)
{
	GuiWindow* window = (GuiWindow*)component;

	if (!window->generalProperties.isVisible)
		return; // Avoid drawing invisible controls

	Uint32 color = SDL_MapRGB(window->surface->format, window->bgColor.r, window->bgColor.g, window->bgColor.b);
	SDL_FillRect(window->surface, NULL, color); // Clear window before beginning to draw (with background color)

	const Rectangle* const parentBounds = &window->generalProperties.bounds;
	window->generalProperties.visibleBounds = window->generalProperties.bounds;
	drawChildComponents(window->subComponents, parentBounds);

	// Swap buffers to show the new drawen stuff on screen
	if (SDL_Flip(window->surface) != 0)
	{
		g_guiError = true;
		printf("ERROR: failed to flip SDL framebuffers: %s\n", SDL_GetError());
	}
}

/** Calculates the absolute coordinates of a control in the window, after taking into consideration the coordinates of the
 *	container parent (e.g: Panel may shift controls, and they may be partly hidden).
 *	compBounds - The relative bounds of the component, relative to the parent.
 *	container - The bounding box of the parent, in absolute coordinates (relative to the window (0, 0)).
 */
const Rectangle getAbsoluteBounds(Rectangle* compBounds, const Rectangle* const container)
{
	int absX = compBounds->x + container->x;
	int absY = compBounds->y + container->y;

	// Contains the amount of pixels we cut from the bounds if the control doesn't fit into the container
	int widthCutoff = maxi(0, (absX + compBounds->width) - (container->x + container->width));
	int absWidth = maxi(0, compBounds->width - widthCutoff); // widthCutoff is [0, inf)
	int heightCutoff = maxi(0, (absY + compBounds->height) - (container->y + container->height));
	int absHeight = maxi(0, compBounds->height - heightCutoff); // heightCutoff is [0, inf)

	Rectangle absoluteBounds = { absX, absY, absWidth, absHeight };

	return absoluteBounds;
}

/** Draws the panel and all sub components inside it. */
void drawPanel(void* component, const Rectangle* const container)
{
	GuiPanel* panel = (GuiPanel*)component;

	if (!panel->generalProperties.isVisible)
		return; // Avoid drawing invisible controls

	const Rectangle absoluteBounds = getAbsoluteBounds(&panel->generalProperties.bounds, container);
	panel->generalProperties.visibleBounds = absoluteBounds; // Keep results in visibleBounds

	if ((panel->generalProperties.visibleBounds.width > 0) &&
		(panel->generalProperties.visibleBounds.height > 0))
	{
		SDL_Rect sdlRect = { absoluteBounds.x, absoluteBounds.y, absoluteBounds.width, absoluteBounds.height };
		SDL_Surface* windowSurface = panel->generalProperties.window->surface;
		Uint32 color = SDL_MapRGB(windowSurface->format, panel->bgColor.r, panel->bgColor.g, panel->bgColor.b);
		SDL_FillRect(windowSurface, &sdlRect, color); // Fill panel background
	}

	// Always draw child components, to update their visible bounds even for invisible controls
	drawChildComponents(panel->subComponents, &absoluteBounds);
}

/** Draws the image and all sub components inside it. */
void drawImage(void* component, const Rectangle* const container)
{
	GuiImage* image = (GuiImage*)component;

	if (!image->generalProperties.isVisible)
		return; // Avoid drawing invisible controls

	Uint32 transparentColor = SDL_MapRGB(image->surface->format,
										 image->transparentColor.r,
										 image->transparentColor.g,
										 image->transparentColor.b);

	// Set transparent color
	if (SDL_SetColorKey(image->surface, SDL_SRCCOLORKEY, transparentColor) != 0)
	{
		g_guiError = true;
		printf("ERROR: failed to set SDL color key (transparent image background color): %s\n", SDL_GetError());
	}

	Rectangle relativeBounds = { image->generalProperties.bounds.x,
								 image->generalProperties.bounds.y,
								 image->scissorRegion.width,
								 image->scissorRegion.height };
	const Rectangle absoluteBounds = getAbsoluteBounds(&relativeBounds, container);

	SDL_Rect imageScissor = { image->scissorRegion.x, image->scissorRegion.y,
							  absoluteBounds.width, absoluteBounds.height }; // Which part of the image to paint
	SDL_Rect target = { absoluteBounds.x, absoluteBounds.y, 0, 0 }; // Where the image is positioned

	// Keep results in visibleBounds
	image->generalProperties.visibleBounds.x = absoluteBounds.x;
	image->generalProperties.visibleBounds.y = absoluteBounds.y,
	image->generalProperties.visibleBounds.width = absoluteBounds.width;
	image->generalProperties.visibleBounds.height = absoluteBounds.height;

	if ((image->generalProperties.visibleBounds.width > 0) &&
		(image->generalProperties.visibleBounds.height > 0))
	{
		// Draw the image to the window's surface
		int blit = SDL_BlitSurface(image->surface, &imageScissor, image->generalProperties.window->surface, &target);

		if (0 != blit)
		{ // Query blit successness
			g_guiError = true;
			printf("ERROR: failed to blit SDL surface while drawing image: %s\n", SDL_GetError());
			SDL_FreeSurface(image->surface);
			image->surface = NULL;
		}
	}
}

/** Draws the button and all sub components inside it. */
void drawButton(void* component, const Rectangle* const container)
{
	GuiButton* button = (GuiButton*)component;

	if ((!button->generalProperties.isVisible) || (NULL == button->bgImage))
		return; // Avoid drawing invisible controls

	// The button's image is actually composed of 4 images of the 4 states.
	// To get the real dimensions we therefore have to divide by 2 both dimensions.
	// We assume the dimensions are even.
	int imageWidth = button->bgImage->generalProperties.bounds.width / 2;
	int imageHeight = button->bgImage->generalProperties.bounds.height / 2;
	int imageX;
	int imageY;

	ButtonState respondState = button->state;
	if (!button->isEnabled)
		respondState = DEFAULT; // Disabled buttons don't show any animation

	// Assume the button's image is composed as follows - we pick the correct state image:
	//
	//  Default     | Mouse Move
	// -------------------------
	//  Mouse Down  | Mouse Up
	//
	switch (respondState)
	{
		case DEFAULT:
		{
			imageX = 0;
			imageY = 0;
			break;
		}
		case MOUSE_MOVE:
		{
			imageX = imageWidth;
			imageY = 0;
			break;
		}
		case MOUSE_DOWN:
		{
			imageX = 0;
			imageY = imageHeight;
			break;
		}
		case MOUSE_UP:
		{
			imageX = imageWidth;
			imageY = imageHeight;
			break;
		}
	}

	button->bgImage->scissorRegion.x = imageX;
	button->bgImage->scissorRegion.y = imageY;
	button->bgImage->scissorRegion.width = imageWidth;
	button->bgImage->scissorRegion.height = imageHeight;

	if (button->isSurfaceOwner) // Drawing method depends if the bg image is shared by multiple controls or not
	{
		// Since the button is drawn as image only, let the image deal with fitting inside the container
		drawImage(button->bgImage, container);
	}
	else
	{
		// The bg image is  a shared resource and therefore may be invisible.
		// Since the button is displayed, is overrides the image's visibility state, so we set it to true
		// and make sure to restore the original value in the end. Same goes for the bounds update.
		bool isBGImageVisible = button->bgImage->generalProperties.isVisible;
		Rectangle originalBounds = button->bgImage->generalProperties.bounds;
		button->bgImage->generalProperties.isVisible = true;
		button->bgImage->generalProperties.bounds = button->generalProperties.bounds;

		// Since the button is drawn as image only, let the image deal with fitting inside the container
		drawImage(button->bgImage, container);

		button->bgImage->generalProperties.isVisible = isBGImageVisible;
		button->bgImage->generalProperties.bounds = originalBounds;
	}

	// Keep results in visibleBounds
	button->generalProperties.visibleBounds = button->bgImage->generalProperties.visibleBounds;
}

/** Draws the animation and all sub components inside it. */
void drawAnimation(void* component, const Rectangle* const container)
{
	GuiAnimation* animation = (GuiAnimation*)component;

	if (!animation->generalProperties.isVisible)
		return; // Avoid drawing invisible controls

	int numOfClipsPerRow = animation->clips->generalProperties.bounds.width / animation->clipWidth;
	int numOfClipsPerCol = animation->clips->generalProperties.bounds.height / animation->clipHeight;
	int totalNumberOfClips = numOfClipsPerCol * numOfClipsPerRow;

	// For non-repeatable animations, check if we're stuck on the last frame
	if ((!animation->isRepeated) && (animation->currentClip == totalNumberOfClips))
	{
		// Since the animation is drawn as image only, let the image deal with fitting inside the container
		drawImage(animation->clips, container);
		return;
	}

	// First - check which clip are we on, if we should change to the next clip, etc.
	if (0 == animation->clipChangeStartTime)
	{ // First clip - define scissor region on first sprite
		animation->currentClip = 1;
		animation->clips->scissorRegion.x = 0;
		animation->clips->scissorRegion.y = 0;
		animation->clips->scissorRegion.width = animation->clipWidth;
		animation->clips->scissorRegion.height = animation->clipHeight;
		animation->clipChangeStartTime = SDL_GetTicks();
	}
	else
	{
		int currTime = SDL_GetTicks();

		if (currTime - animation->clipChangeStartTime > animation->timeBetweenFramesMs)
		{ // Advance to next sprite

			animation->currentClip++;
			animation->clips->scissorRegion.x += animation->clipWidth;

			// Check if should advance to the next row
			if (animation->clips->scissorRegion.x >= animation->clips->generalProperties.bounds.width)
			{
				animation->clips->scissorRegion.x = 0;
				animation->clips->scissorRegion.y += animation->clipHeight;

				// Check if we've reached the animation end
				if (animation->clips->scissorRegion.y >= animation->clips->generalProperties.bounds.height)
				{
					if (animation->isRepeated)
					{ // Return to first sprite
						animation->currentClip = 1;
						animation->clips->scissorRegion.x = 0;
						animation->clips->scissorRegion.y = 0;
					}
					else
					{ // Remain on the last sprite
						animation->currentClip = totalNumberOfClips;
						animation->clips->scissorRegion.x = (numOfClipsPerRow - 1) * animation->clipWidth;
						animation->clips->scissorRegion.y = (numOfClipsPerCol - 1) * animation->clipHeight;
					}

					// Prompt animation end event if one exists
					if (NULL != animation->onAnimationEnd)
						animation->onAnimationEnd(animation);
				}
			}

			animation->clipChangeStartTime = currTime;
		}
	}

	// Since the animation is drawn as image only, let the image deal with fitting inside the container
	drawImage(animation->clips, container);

	// Keep results in visibleBounds
	animation->generalProperties.visibleBounds = animation->clips->generalProperties.visibleBounds;
}

/** Draws the dialog and all sub components inside it. */
void drawDialog(void* component, const Rectangle* const container)
{
	GuiDialog* dialog = (GuiDialog*)component;

	if (!dialog->generalProperties.isVisible)
		return; // Avoid drawing invisible controls

	drawPanel(dialog->dialogPanel, container); // Dialog is painted by the sub-panel

	dialog->generalProperties.visibleBounds = dialog->dialogPanel->generalProperties.visibleBounds;
}

//  ------------------------------ 
//  -- Event handling           --
//  ------------------------------

/** Returns whether the component represented by the wrapper is under the mouse cursor (x,y).
 *  This functions uses the component's visible bounds which are absoulte coordinates (match window (0,0)).
 */
bool isComponentUnderMouse(GuiComponentWrapper* wrapper, int mouseX, int mouseY)
{
	GuiGeneralProperties* componentProperties = getComponentGeneralProperties(wrapper);
	bool isUnderMouse =
		(componentProperties->visibleBounds.x < mouseX) &&
		(componentProperties->visibleBounds.x + componentProperties->visibleBounds.width > mouseX) &&
		(componentProperties->visibleBounds.y < mouseY) &&
		(componentProperties->visibleBounds.y + componentProperties->visibleBounds.height > mouseY);

	return isUnderMouse && componentProperties->isVisible;
}

/** Performs a hit test and searchs for a clickable button under the given mouse coordinates.
 *	If no button was found, NULL will be returned.
 *	Every time this method is called the entire UI Tree is scanned in a DFS manner, even if a panel falls out
 *  of the mouse coordinates, and this is due to the second purpose of this function - it resets the state
 *	of every button in the tree to DEFAULT (and this way we avoid the "onMouseMoveOut" event that doesn't exist in SDL,
 *	The state of the hit button is expected to be handled by the caller of the hitTest).
 */
GuiComponentWrapper* hitTestAndPrepareUITree(int mouseX, int mouseY, LinkedList* components)
{
	GuiComponentWrapper* result = NULL;
	Node* currNode = components->head;

	// We search inside containers (panels, windows)
	// Only clickable elements are sensitive to hit test (e.g: buttons. Images and animations are not)
	while (NULL != currNode)
	{
		GuiComponentWrapper* wrapper = (GuiComponentWrapper*)currNode->data;
		
		if (wrapper->type == PANEL)
		{ // Perform hit test for inner components
			GuiComponentWrapper* innerResult = 
				hitTestAndPrepareUITree(mouseX, mouseY, ((GuiPanel*)wrapper->component)->subComponents);

			if (isComponentUnderMouse(wrapper, mouseX, mouseY))
				result = NULL; // The panel hides all components found so far

			if (NULL != innerResult)
			{ // Only update if hit test was successful, so inner hit tests that fail don't override sibling's successful ones
				result = innerResult;
			}
		}
		else if (wrapper->type == DIALOG)
		{ // Perform hit test for inner panel component

			GuiPanel* dialogPanel = ((GuiDialog*)wrapper->component)->dialogPanel;
			GuiComponentWrapper* innerResult =
				hitTestAndPrepareUITree(mouseX, mouseY, dialogPanel->subComponents);

			if (isComponentUnderMouse(dialogPanel->generalProperties.wrapper, mouseX, mouseY))
				result = NULL; // The panel hides all components found so far

			if (NULL != innerResult)
			{ // Only update if hit test was successful, so inner hit tests that fail don't override sibling's successful ones
				result = innerResult;
			}
		}
		else if ((wrapper->type == BUTTON) || (wrapper->type == DIALOG_BUTTON))
		{
			GuiButton* button = (GuiButton*)wrapper->component;
			button->state = DEFAULT; // Reset the state of every button encountered

			bool isButtonUnderMouse = isComponentUnderMouse(wrapper, mouseX, mouseY);

			// A button is clickable if it has an "onClick" event attached to it and it is under the mouse and enabled
			if (isButtonUnderMouse)
			{
				if ((NULL != button->onClick) && button->isEnabled && (button->bgImage != NULL))
				{
					result = wrapper;
				}
				else
				{
					result = NULL; // The button is not clickable but hides previous controls
				}
			}
		}
		else
		{
			if (isComponentUnderMouse(wrapper, mouseX, mouseY))
				result = NULL; // All other components don't respond to hit test but may hide components that do
		}

		currNode = currNode->next;
	}

	// Return last hit test results (for component with highest z order)
	return result;
}

/** Queries which SDL events have happened and prompts corresponding events in the Gui FW.
 *  (e.g: GuiButton was clicked). This implementation supports windows and modal dialogs alike.
 */
bool processGuiEventsForBranch(GuiComponentWrapper* wrapper)
{
	SDL_Event e;
	bool isQuit = false;

	// Get the window and ui branch we're processing events for
	// For windows its all the components in the window.
	// For dialog we query only components that reside under the dialog
	GuiWindow* activeWindow = NULL;
	LinkedList* uiBranch = NULL;
	if (wrapper->type == WINDOW)
	{
		activeWindow = (GuiWindow*)wrapper->component;
		uiBranch = activeWindow->subComponents;
	}
	else
	{
		activeWindow = ((GuiDialog*)wrapper->component)->generalProperties.window;
		uiBranch = ((GuiDialog*)wrapper->component)->dialogPanel->subComponents;
	}

	// Check if event have happened
	while (0 != SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case (SDL_QUIT) :
			{ // Quit event
				isQuit = true;
				activeWindow->isWindowQuit = true;
				break;
			}
			case (SDL_MOUSEMOTION) :
			{ // Mouse move event - relevant for buttons
				GuiComponentWrapper* hitComp = hitTestAndPrepareUITree(e.motion.x, e.motion.y, uiBranch);

				if (NULL != hitComp)
				{ // A component is under the mouse
					if ((BUTTON == hitComp->type) || (DIALOG_BUTTON == hitComp->type))
					{ // And it is a button
						((GuiButton*)hitComp->component)->state = MOUSE_MOVE;
					}
				}

				break;
			}
			case (SDL_MOUSEBUTTONDOWN) :
			case (SDL_MOUSEBUTTONUP) :
			{ // Mouse down event, relevant for buttons or
				// Mouse up event, relevant for buttons

				GuiComponentWrapper* hitComp = hitTestAndPrepareUITree(e.button.x, e.button.y, uiBranch);

				if (NULL != hitComp)
				{ // A component is under the mouse
					if ((BUTTON == hitComp->type) || (DIALOG_BUTTON == hitComp->type))
					{ // And it is a button
						ButtonState clickableState = (e.type == SDL_MOUSEBUTTONDOWN) ? MOUSE_DOWN : MOUSE_UP;
						GuiButton* button = (GuiButton*)hitComp->component;
						button->state = clickableState; // Update the button state

						// For onMouseUp - call the onClick event of the button
						if ((e.type == SDL_MOUSEBUTTONUP) && (NULL != button->onClick))
						{
							button->onClick(button);
						}
					}
				}

				break;
			}
			default:
			{
				break;
			}
		}
	}

	return isQuit;
}

/** Queries which SDL events have happened and prompts corresponding events in the Gui FW.
 *  (e.g: GuiButton was clicked).
 */
bool processGuiEvents(GuiWindow* activeWindow)
{
	return processGuiEventsForBranch(activeWindow->generalProperties.wrapper);
}

//  ------------------------------ 
//  -- Misc functions           --
//  ------------------------------

/** Initializes the SDL part of the Gui FW, and perpares the system for creation of Gui components. */
int initGui()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("ERROR: unable to init SDL: %s\n", SDL_GetError());
		return SDL_FAILURE_EXIT_CODE;
	}

	atexit(SDL_Quit); // Make sure SDL quits upon program termination
	return OK_EXIT_CODE;
}