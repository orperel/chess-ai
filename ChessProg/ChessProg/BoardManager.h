#ifndef BOARD_MANAGER_
#define BOARD_MANAGER_

#include <stdio.h>
#include "Types.h"
#include "Chess.h"

void init_board(char board[BOARD_SIZE][BOARD_SIZE]);

void print_board(char board[BOARD_SIZE][BOARD_SIZE]);

/* Clear the Chess board (remove all the pieces). */
void clearBoard(char board[BOARD_SIZE][BOARD_SIZE]);

/* A constructor function for Position structs. */
Position* createPosition();

/* A constructor function for Move structs. */
Move* createMove(Position startPos);

/* A deep copy constructor function for Move structs. */
Move* cloneMove(Move* original);

/* A destructor function for Move structs */
void deleteMove(void* move);

/* A constructor function for GameStep structs. */
GameStep* createGameStep(char board[BOARD_SIZE][BOARD_SIZE], Move* move);

/* A destructor function for GameStep structs. */
void deleteGameStep(GameStep* step);

/* Execute game step on the board. */
void doStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

/* Undo game step on the board. */
void undoStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

/* Returns if the square is on the board area. */
bool isSquareOnBoard(int i, int j);

/* Returns if the square is on the board and has no soldiers on it. */
bool isSquareVacant(char board[BOARD_SIZE][BOARD_SIZE], int i, int j);

/* Returns if the square is on the board and occupied by the current player. */
bool isSquareOccupiedByCurrPlayer(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and occupied by the enemy. */
bool isSquareOccupiedByEnemy(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and occupied any king. */
bool isSquareOccupiedByKing(char board[BOARD_SIZE][BOARD_SIZE], int i, int j);

/* Returns if the soldier goes to endPos, will it become a king. */
bool isBecomeKing(char soldier, Position endPos);

/* Returns the remaining army size of black or white player. */
Army getArmy(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackSoldiers);

/* Validate that there are no men in the opponent edge. */
bool validEdges(char board[BOARD_SIZE][BOARD_SIZE]);

#endif