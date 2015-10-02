#ifndef CONSOLE_
#define CONSOLE_

#include "Types.h"
#include "Chess.h"

/* Reads a user input line into g_inputLine as null terminated string. */
void getUserInput();

/*
 * Breaks the g_inputLine into arguments using space as a delimiter.
 * The results will be packed inside args.
 * Calling functions should make sure to free the memory held by the args when done!
 */
int breakInputToArgs(char* args[MAX_ARGS]);

/*
 * Returns the position represented by a <i,j> string tuple.
 * This function also converts from the chess logical representation (letter, digit) to array indices.
 */
Position argToPosition(char* arg);

/*
 * Parse next user setting during Settings state and execute it.
 * Return RETRY if the settings haven't done, QUIT if a quit command was entered
 * and SUCCESS if a success command was entered (and the board initialization is valid).
 */
COMMAND_RESULT parseUserSettings(char board[BOARD_SIZE][BOARD_SIZE]);

 /*
  * Parse next user command during Game state and execute it.
  * Return RETRY if the turn hasn't done yet, QUIT if a quit command was entered
  * and SUCCESS if the command was executed successfully.
  */
COMMAND_RESULT parseUserCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack);

/* A "toString()" function for Move structs (for console). */
void printMove(Move* move);

#endif