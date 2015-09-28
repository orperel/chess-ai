#ifndef GAME_COMMANDS_
#define GAME_COMMANDS_

#include "Types.h"
#include "Chess.h"

#define TAG_LENGTH 30
#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define GAME_TAG_BEGIN "<game>"
#define GAME_TAG_END "</game>"
#define NEXT_TURN_TAG_BEGIN "<next_turn"
#define NEXT_TURN_TAG_END "</next_turn>"
#define GAME_MODE_TAG_BEGIN "<game_mode"
#define GAME_MODE_TAG_END "</game_mode>"
#define DIFFICULTY_TAG_BEGIN "<difficulty"
#define DIFFICULTY_TAG_END "</difficulty>"
#define USER_COLOR_TAG_BEGIN "<user_color"
#define USER_COLOR_TAG_END "</user_color>"
#define BOARD_TAG_BEGIN "<board"
#define BOARD_TAG_END "</board>"
#define ROW_TAG_BEGIN "<row_"
#define ROW_TAG_END "</row_"
#define WRONG_FORMAT "Wrong XML format\n"

/* A "toString()" function for Move structs. */
void printMove(Move* move);

/*
 * Parses and builds "Move" struct out of the arguments.
 * The Move will also be validated. If it is illegal, NULL is returned.
 * If the function succeeds and this is a valid move, the move is returned.
 */
Move* parseAndBuildMove(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* args[]);

/*
 * Executes a move command done by the user.
 * This function parses, builds, validates and executes the move.
 * Returns True if the move is legal and executed successfully.
 * If this move is impossible, False is returned.
 */
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* args[MAX_ARGS]);

/* Prints a list of all possible moves for the player. */
void executeGetMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* arg);

/*
 * Return the score for the given move in a minimax tree of the given depth.
 * If there was an allocation error return INT_MIN.
 */
int executeGetScoreCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* depth, Move* move);

/*
 * Print all the moves with the highest score for the current board.
 * 'depth' is an argument for the minimax algorithm.
 */
void executeGetBestMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* depth);

/*
 * Load the game settings from the file "path", path being the full or relative path to the file.
 * We assume that the file contains valid data and is correctly formatted.
 */
void executeLoadCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path);

/* Save the current game state to the file "path". */
void executeSaveCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path);

#endif