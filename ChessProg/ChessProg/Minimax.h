#ifndef MINIMAX_
#define MINIMAX_

#include "Types.h"

#define PAWN_SCORE 1
#define BISHOP_SCORE 3
#define ROOK_SCORE 5
#define KNIGHT_SCORE 3
#define QUEEN_SCORE 9
#define KING_SCORE 400
#define WINNING_SCORE 1000
#define LOOSING_SCORE -1000
#define TIE_SCORE_ABS 200

int getScore(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack);

int alphabeta(char board[BOARD_SIZE][BOARD_SIZE], int level, int alpha, int beta, bool isABlack);

Move* minimax(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack);

#endif