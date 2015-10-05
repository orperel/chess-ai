#ifndef GAME_COMMANDS_
#define GAME_COMMANDS_

#include "Types.h"
#include "Chess.h"

#define DIFFICULTY_BEST_INT -1

#define TAG_LENGTH 50
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


/** Returns true if the move is a legal move by the given player (black or white).
 *  Validation is done by comparing the move to all legal moves, so make sure to query the mem flag on return.
 */
bool validateMove(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, Move* move);

/*
 * Executes a move command done by the user.
 * Move is expected to be validated with validateMove() before calling this function.
 * This function safely executes the move.
 * Returns True if the move is legal and executed successfully.
 * If this move failed to execute (due to an unexpected memory error), False is returned.
 */
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, Move* move);

/* Returns a list of all possible moves for the player for one square.
 * List must be freed by user when usage terminates.
 */
LinkedList* executeGetMovesForPosCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, Position pos);

/*
 * Return the score for the given move in a minimax tree of the given depth.
 * If there was an allocation error return INT_MIN.
 */
int executeGetScoreCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, int depth, Move* move);

/*
 * Return all the moves with the highest score for the current board.
 * 'depth' is an argument for the minimax algorithm.
 * List of moves must be freed when usage is complete.
 */
LinkedList* executeGetBestMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, int depth);

/*
 * Load the game settings from the file "path", path being the full or relative path to the file.
 * We assume that the file contains valid data and is correctly formatted.
 */
void executeLoadCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path);

/* Save the current game state to the file "path". */
void executeSaveCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path);

#endif