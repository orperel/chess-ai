#ifndef GAME_LOGIC_
#define GAME_LOGIC_

#include <stdio.h>
#include "LinkedList.h"
#include "Types.h"
#include "Chess.h"

/* 
 * Iterates the board and returns a list of moves the player can make with each soldier
 * Input:
 *		board ~ The game board.
 *	    isMovesForBlackPlayer ~ True if the function returns moves for the black player.
 *								False if the function returns moves for the white player.
 */
LinkedList* getMoves(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer);

/* 
 * Iterates the board and returns whether the player can make any more move (false) or he is stuck (true).
 * This function is more optimized than getMoves() - it will stop as soon as possible moves are found.
 * Also - chained eats won't be checked.
 * Input:
 *		board ~ The game board.
 *	    isBlackPlayer ~ True if the function checks moves for the black player.
 *						False if the function checks moves for the white player.
 */
bool isPlayerStuck(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPlayer);

/*
* Checks the board to see if the black / white player has won.
* Input:
*		board ~ The game board.
*		isPlayerBlack ~ Check if the black player has won (true) or if the white player has won (false).
*/
bool isPlayerVictor(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack);

#endif