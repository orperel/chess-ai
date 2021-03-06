#ifndef GAME_COMMANDS_
#define GAME_COMMANDS_

#include "Types.h"
#include "LinkedList.h"

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

/** Enum for the various states in chess games. */
typedef enum
{
	GAME_ONGOING,
	GAME_CHECK,
	GAME_MATE_BLACK_WINS,
	GAME_MATE_WHITE_WINS,
	GAME_TIE,
	GAME_ERROR
} ChessGameState;

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
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], Move* move);

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

/* Fetch the next turn done by the computer. */
Move* executeGetNextComputerMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack);

/* Check for checkmate or a tie and return the state of the board.
 * Note: mate or tie are termination cases.
 */
ChessGameState executeCheckMateTieCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isBlack);

/*
 * Load the game settings from the file "path", path being the full or relative path to the file.
 * We assume that the file contains valid data and is correctly formatted.
 * Return True if the loading ended successfully, else False.
 */
bool executeLoadCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path);

/* 
 * Save the current game state to the file "path". 
 * Return True if the saving ended successfully, else False.
 */
bool executeSaveCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path, bool isBlackTurn);

/* Validate board initialization. If it is valid the program can move to game state. */
bool isValidStart(char board[BOARD_SIZE][BOARD_SIZE]);

#endif