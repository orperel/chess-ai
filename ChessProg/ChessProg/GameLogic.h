#ifndef GAME_LOGICS_
#define GAME_LOGICS_

#include <stdio.h>
#include "Chess.h"
#include "Types.h"
#include "LinkedList.h"

typedef struct
{
	int men;
	int kings;
} Army;

/* Returns if the square is on the board area. */
bool isSquareOnBoard(int i, int j);

/* Returns if the square is on the board and occupied by the enemy. */
bool isSquareOccupiedByEnemy(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and occupied by the current player. */
bool isSquareOccupiedByCurrPlayer(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and has no soldiers on it. */
bool isSquareVacant(char board[BOARD_SIZE][BOARD_SIZE], int i, int j);

/* Returns if the square is on the board and occupied any king. */
bool isSquareOccupiedByKing(char board[BOARD_SIZE][BOARD_SIZE], int i, int j);

/* Returns -- if the soldier goes to endPos, will it become a king. */
bool isBecomeKing(char soldier, Position endPos);

/* Returns the remaining army size of black or white player. */
Army getArmy(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackSoldiers);

/* Checks the board to see if the black / white player has won.
*  Input:
*		board - The game board.
*		isPlayerBlack ~ Check if the black player has won (true) or if the white player has won (false).
*/
bool isPlayerVictor(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack);

/* Iterates the board and returns a list of moves the player can make with each soldier
 * Input:
 *		board - The game board.
 *	    isMovesForBlackPlayer - True if the function returns moves for the black player.
 *								False if the function returns moves for the white player.
 */
LinkedList* getMoves(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer);

/* Iterates the board and returns whether the player can make any more move (false) or he is stuck (true).
* This function is more optimized than getMoves() - it will stop as soon as possible moves are found.
* Also - chained eats won't be checked.
* Input:
*		board - The game board.
*	    isBlackPlayer - True if the function checks moves for the black player.
*						False if the function checks moves for the white player.
*/
bool isPlayerStuck(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPlayer);

/** A constructor function for GameStep structs. */
GameStep* createGameStep(char board[BOARD_SIZE][BOARD_SIZE], Move* move);

/** A destructor function for GameStep structs */
void deleteGameStep(GameStep* step);

/** Execute game step on the board. */
void doStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

/** Undo game step on the board. */
void undoStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

#endif