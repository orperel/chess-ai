//  -----------------------------------------------
//  -- Common Type definitions for chess gui     --
//  -----------------------------------------------

#define WIN_W 640	// Window dimensions
#define WIN_H 480
#define BUTTON_W 200 // Dimensions for buttons
#define BUTTON_H 80
#define DIALOG_BGIMAGE "Resources/dialog_background.bmp" // Dialog background image (dynamic image size is ok)
#define BUTTON_CANCEL_IMG "Resources/button_cancel.bmp"

#define NUM_OF_SAVE_SLOTS 7 // Number of slots for saving / loading
#define SAVE_GAME_PATH "save_slot" // Generic path for save-games. We add an additional digit for identification of slot
#define SAVE_FILE_EXTENSION ".xml" // File extension of all save files
#define SAVE_SLOT_PATH "Resources/button_slot" // Image of "Slot #" button
#define RESOURCE_IMG_EXT ".bmp"
#define MINMAX_DEPTH_IMG_PATH "Resources/button_depth" // Image of "Depth #" button
#define MINMAX_BEST_DEPTH_IMG_PATH "Resources/button_depthbest.bmp" // Image of "BEST DEPTH" button