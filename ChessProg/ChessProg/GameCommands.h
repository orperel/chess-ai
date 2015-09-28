#ifndef GAME_COMMANDS_
#define GAME_COMMANDS_

#include "Types.h"
#include "Chess.h"

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

#endif