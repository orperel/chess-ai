#ifndef MINIMAX_
#define MINIMAX_

#include "Types.h"
#include "Chess.h"

#define WINNING_SCORE 100
#define LOOSING_SCORE -100
#define MAN_SCORE 1
#define KING_SCORE 3

int getScore(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack);

int alphabeta(char board[BOARD_SIZE][BOARD_SIZE], int level, int alpha, int beta, bool isABlack);

Move* minimax(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack);

#endif