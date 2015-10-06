#ifndef CONSOLE_
#define CONSOLE_

#include "Types.h"

#define print_message(message) (printf("%s", message));

/*
 * Initiates the console game loop.
 * The two users, or the computer in Player Vs. AI mode, each take turns until one of them wins or the user quits.
 * This function runs an entire single Chess game, after initializing the board and determining game settings.
 */
int initConsoleMainLoop();

#endif