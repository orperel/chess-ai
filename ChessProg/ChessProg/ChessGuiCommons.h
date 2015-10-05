#ifndef CHESS_GUI_COMMONS_
#define CHESS_GUI_COMMONS_

#include "GuiFW.h"

//  -----------------------------------------------
//  -- Common Type definitions for chess gui     --
//  -----------------------------------------------

#define GAME_WINDOW_TITLE "Chess game"
#define WIN_W 640	// Window dimensions
#define WIN_H 480
#define BUTTON_W 200 // Dimensions for buttons
#define BUTTON_H 80
#define DIALOG_BGIMAGE "Resources/dialog_background.bmp" // Dialog background image (dynamic image size is ok)
#define BUTTON_CANCEL_IMG "Resources/button_cancel.bmp"
#define BUTTON_MENU_IMG "Resources/button_mainMenu.bmp"
#define BUTTON_QUIT_IMG "Resources/button_quit.bmp"
#define BUTTON_QUIT_SMALL_IMG "Resources/button_quit_small.bmp"

#define NUM_OF_SAVE_SLOTS 7 // Number of slots for saving / loading
#define SAVE_GAME_PATH "save_slot" // Generic path for save-games. We add an additional digit for identification of slot
#define SAVE_FILE_EXTENSION ".xml" // File extension of all save files
#define SAVE_SLOT_PATH "Resources/button_slot" // Image of "Slot #" button
#define RESOURCE_IMG_EXT ".bmp"
#define MINMAX_DEPTH_IMG_PATH "Resources/button_depth" // Image of "Depth #" button
#define MINMAX_BEST_DEPTH_IMG_PATH "Resources/button_depthbest.bmp" // Image of "BEST DEPTH" button

/** Show the "depth" dialog, for choosing a depth for minmax.
 *	The depth chosen by the user is returned.
 *	DIFFICULTY_BEST_INT is returned if BEST is chosen.
 *	(DIFFICULTY_BEST_INT - 1) is returned if the dialog is canceled.
 *	(DIFFICULTY_BEST_INT - 2) is returned on error.
 */
int showDepthDialog(GuiWindow* window);

/** Open the save / load game to slots dialog.
 *	The save slot path chosen by the user is returned (and should be freed by the caller).
 *	NULL is returned if the dialog is canceled or an error occured (for errors, global flags are set).
 */
char* showLoadSaveDialog(GuiWindow* window);

#endif