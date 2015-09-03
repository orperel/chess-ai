#ifndef GAME_LOGIC_
#define GAME_LOGIC_

#include <stdio.h>
#include "LinkedList.h"
#include "Types.h"
#include "Chess.h"

/*
 * Get all possible moves for the given square.
 * If the square is illegal or vacant, empty list is returned.
 * Otherwise we query the square to find out which player occupies the square, and returns the possible moves for that
 * player's piece.
 * Input:
 *		board ~ The game board.
 *		x, y ~ The position on board to search for moves.
 */
LinkedList* getMovesForSquare(char board[BOARD_SIZE][BOARD_SIZE], int x, int y);

/*
 * Iterates the board and returns a list of moves the player can make with each piece
 * Input:
 *		board ~ The game board.
 *		isMovesForBlackPlayer ~ True if the function returns moves for the black player.
 *							    False if the function returns moves for the white player.
 */
LinkedList* getMoves(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer);

/*
 * Returns either whether the black player (isTestForBlackPlayer == true) is in check,
 * or the white player (isTestForBlackPlayer == false) is in check.
 */
bool isCheck(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer);

/* Returns if the black player (isTestForBlackPlayer-true) or white player (isTestForBlackPlayer=false)
 *	are in Matt (their king is in danger and cannot be saved).
 *	To optimize, this method accepts possibleMoves for the given player, to avoid calculating them all over again.
 */
bool isMatt(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, LinkedList* possibleMoves);

/* Returns if the black player (isTestForBlackPlayer-true) or white player (isTestForBlackPlayer=false)
 *	are in tie (their king is not in danger but no additional moves can be made).
 *	To optimize, this method accepts possibleMoves for the given player, to avoid calculating them all over again.
 */
bool isTie(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, LinkedList* possibleMoves);

#endif