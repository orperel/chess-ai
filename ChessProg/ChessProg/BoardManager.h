#ifndef BOARD_MANAGER_
#define BOARD_MANAGER_

#include <stdio.h>
#include "Types.h"

/* Init the board with the pieces in the beginning of a game. */
void init_board(char board[BOARD_SIZE][BOARD_SIZE]);

/* Print to console the state of the board. */
void print_board(char board[BOARD_SIZE][BOARD_SIZE]);

/* Clear the Chess board (remove all the pieces). */
void clearBoard(char board[BOARD_SIZE][BOARD_SIZE]);

/* A constructor function for Position structs. */
Position* createPosition();

/* A constructor function for Move structs. */
Move* createMove(Position* startPos, Position* targetPos);

/* A deep copy constructor function for Move structs. */
Move* cloneMove(Move* original);

/* A destructor function for Move structs */
void deleteMove(void* move);

/* Execute move on the board. In the end of this function the move is deleted. */
bool executeMove(char board[BOARD_SIZE][BOARD_SIZE], Move* move);

/* A constructor function for GameStep structs. */
GameStep* createGameStep(char board[BOARD_SIZE][BOARD_SIZE], Move* move);

/* A destructor function for GameStep structs. */
void deleteGameStep(GameStep* step);

/* Execute game step on the board. */
void doStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

/* Undo game step on the board. */
void undoStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step);

/*
 * Returns the size of the army of the black / white player (according to the input isBlackSoldiers parameter).
 * Results detail how many of each piece remain.
 */
Army getArmy(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackSoldiers);

/* Validate that there are no men in the opponent edge. */
bool validEdges(char board[BOARD_SIZE][BOARD_SIZE]);

/* Returns the white / black king's position. */
Position getKingPosition(char board[BOARD_SIZE][BOARD_SIZE], bool isSearchBlackKing);

/* Returns if the square is on the board area. */
bool isSquareOnBoard(int i, int j);

/* Returns if the square is on the board and has no soldiers on it. */
bool isSquareVacant(char board[BOARD_SIZE][BOARD_SIZE], int i, int j);

/* Returns if the square is on the board and occupied by the current player. */
bool isSquareOccupiedByCurrPlayer(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and occupied by the enemy. */
bool isSquareOccupiedByEnemy(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j);

/* Returns if the square is on the board and occupied by the a pawn of the given color. */
bool isSquareOccupiedByPawn(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the board and occupied by the a bishop of the given color. */
bool isSquareOccupiedByBishop(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the board and occupied by the a rook of the given color. */
bool isSquareOccupiedByRook(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the board and occupied by the a knight of the given color. */
bool isSquareOccupiedByKnight(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the board and occupied by the a queen of the given color. */
bool isSquareOccupiedByQueen(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the board and occupied by the a king of the given color. */
bool isSquareOccupiedByKing(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j);

/* Returns if the square is on the edge of the board on the "enemy's side" (bottom for black, top for white). */
bool isSquareOnOppositeEdge(bool isBlackPiece, int row);

/** Returns true if the square is occupied by a piece of the black player. */
bool isSquareOccupiedByBlackPlayer(char board[BOARD_SIZE][BOARD_SIZE], int x, int y);

/** Returns true if the square is occupied by a piece of the white player. */
bool isSquareOccupiedByWhitePlayer(char board[BOARD_SIZE][BOARD_SIZE], int x, int y);

/* Return True if the square <i,j> is a black square, else False. */
bool isBlackSquare(int i, int j);

#endif