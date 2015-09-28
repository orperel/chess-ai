#include <stdlib.h>
#include "LinkedList.h"
#include "Chess.h"
#include "BoardManager.h"

/* Init the board with the pieces in the beginning of a game. */
void init_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;	// i = column, j = row
	for (i = 0; i < BOARD_SIZE; ++i)
	{	
		switch (i)
		{
			case(0):
			case(7):
			{
				// Init rooks
				board[i][0] = WHITE_R;
				board[i][7] = BLACK_R;
				break;
			}
			case(1):
			case(6):
			{
				// Init knights
				board[i][0] = WHITE_N;
				board[i][7] = BLACK_N;
				break;
			}
			case(2):
			case(5):
			{
				// Init bishops
				board[i][0] = WHITE_B;
				board[i][7] = BLACK_B;
				break;
			}
			case(3):
			{ // Init kings
				board[i][0] = WHITE_K;
				board[i][7] = BLACK_K;
				break;
			}
			case(4):
			{ // Init queens
				board[i][0] = WHITE_Q;
				board[i][7] = BLACK_Q;
				break;
			}
			default:
			{ // Illegal case
				break;
			}
		}

		// Init pawns
		board[i][1] = WHITE_P;
		board[i][6] = BLACK_P;
	}

	// Init empty squares
	for (i = 0; i < BOARD_SIZE; ++i)
		for (j = 2; j < BOARD_SIZE-2; ++j)
			board[i][j] = EMPTY;
}

/* Print to console a single line of the state of the board. */
void print_line(){
	int i;
	printf("  |");
	for (i = 1; i < BOARD_SIZE * 4; i++){
		printf("-");
	}
	printf("|\n");
}

/* Print to console the state of the board. */
void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	print_line();
	for (j = BOARD_SIZE - 1; j >= 0; j--)
	{
		printf((j < 9 ? " %d" : "%d"), j + 1);
		for (i = 0; i < BOARD_SIZE; i++){
			printf("| %c ", board[i][j]);
		}
		printf("|\n");
		print_line();
	}
	printf("   ");
	for (j = 0; j < BOARD_SIZE; j++){
		printf(" %c  ", (char)('a' + j));
	}
	printf("\n");
}

/* Clear the Chess board (remove all the pieces). */
void clearBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			board[i][j] = EMPTY;
		}
	}
}

/* A constructor function for Position structs. */
Position* createPosition(int x, int y)
{
	Position* newPos = (Position*)malloc(sizeof(Position));
	if (newPos == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	newPos->x = x;
	newPos->y = y;

	return newPos;
}

/* A constructor function for Move structs. */
Move* createMove(Position* startPos, Position* targetPos)
{
	Move* newMove = (Move*)malloc(sizeof(Move));
	if (newMove == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	newMove->initPos.x = startPos->x;
	newMove->initPos.y = startPos->y;
	newMove->nextPos.x = targetPos->x;
	newMove->nextPos.y = targetPos->y;
	newMove->promotion = EMPTY; // By default in the general case - no promotion. Manually set if needed.

	return newMove;
}

/* A deep copy constructor function for Move structs. */
Move* cloneMove(Move* original)
{
	Position* originalStartPos = &(original->initPos);
	Position* originalTargetPos = &(original->nextPos);
	Move* clone = createMove(originalStartPos, originalTargetPos);
	if (g_memError)
		return NULL;
	
	clone->promotion = original->promotion; // Copy promotion field manually

	return clone;
}

/* A destructor function for Move structs. */
void deleteMove(void* move)
{
	free(move);
}

/* Execute move on the board. In the end of this function the move is deleted. */
bool executeMove(char board[BOARD_SIZE][BOARD_SIZE], Move* move)
{
	GameStep* nextStep = createGameStep(board, move);
	if (g_memError)
		return false;

	doStep(board, nextStep);

	deleteGameStep(nextStep);
	deleteMove((void*)move);
	return true;
}

/* A constructor function for GameStep structs. */
GameStep* createGameStep(char board[BOARD_SIZE][BOARD_SIZE], Move* move)
{
	GameStep* step = (GameStep*)malloc(sizeof(GameStep));
	if (step == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	bool isBlackPlayer = isSquareOccupiedByBlackPlayer(board, move->initPos.x, move->initPos.y);
	step->isStepByBlackPlayer = isBlackPlayer;
	step->currSoldier = board[move->initPos.x][move->initPos.y];
	step->startPos = move->initPos;
	step->endPos = move->nextPos;
	step->promotion = move->promotion; // Move contains promotion only if it happened for a pawn. We trust its validity here.

	step->isEnemyRemovedInStep = false; // By default, set to false. Next we check if an eat happened and reset accordingly.

	// Check if an enemy was eaten in this step
	if (isSquareOccupiedByEnemy(board, isBlackPlayer, move->nextPos.x, move->nextPos.y))
	{
		step->isEnemyRemovedInStep = true;

		// Eaten soldier is located at where the current piece lands.
		step->removedType = board[move->nextPos.x][move->nextPos.y];
	}

	return step;
}

/* A destructor function for GameStep structs. */
void deleteGameStep(GameStep* step)
{
	free(step);
}

/* Execute game step on the board. */
void doStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step)
{
	// Remove start position
	board[step->startPos.x][step->startPos.y] = EMPTY;

	// Set end position to promotion / normal movement. This also removes an eaten enemy if there was any.
	board[step->endPos.x][step->endPos.y] = (step->promotion != EMPTY) ? step->promotion : step->currSoldier;
}

/* Undo game step on the board. */
void undoStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step)
{
	// Restore the original value of the target square (empty square for movement, eaten piece if there was an eat move).
	board[step->endPos.x][step->endPos.y] = (step->isEnemyRemovedInStep) ? step->removedType : EMPTY;

	// Set start position
	if (EMPTY == step->promotion)
	{ // Piece moved without promotion.
		board[step->startPos.x][step->startPos.y] = step->currSoldier;
	}
	else
	{ // Only pawns can get promoted, so the original square must have been a pawn. Check which color was it.
		board[step->startPos.x][step->startPos.y] = step->isStepByBlackPlayer ? BLACK_P : WHITE_P;
	}
	
}

/* Returns if the square is on the board area. */
bool isSquareOnBoard(int i, int j)
{
	return ((i >= 0) && (j >= 0) && (i < BOARD_SIZE) && (j < BOARD_SIZE));
}

/* Returns if the square is on the board and has no soldiers on it. */
bool isSquareVacant(char board[BOARD_SIZE][BOARD_SIZE], int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (board[i][j] == EMPTY);
}

/* Returns if the square is on the board and occupied by the current player. */
bool isSquareOccupiedByCurrPlayer(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	bool isWhiteMoveAndWhiteOccupies = !isMovesForBlackPlayer &&
										((board[i][j] == WHITE_K) ||
										 (board[i][j] == WHITE_Q) ||
										 (board[i][j] == WHITE_R) ||
										 (board[i][j] == WHITE_N) ||
										 (board[i][j] == WHITE_B) ||
										 (board[i][j] == WHITE_P));

	bool isBlackMoveAndBlackOccupies = isMovesForBlackPlayer &&
										((board[i][j] == BLACK_K) ||
										 (board[i][j] == BLACK_Q) ||
										 (board[i][j] == BLACK_R) ||
										 (board[i][j] == BLACK_N) ||
										 (board[i][j] == BLACK_B) ||
										 (board[i][j] == BLACK_P));

	return (isWhiteMoveAndWhiteOccupies || isBlackMoveAndBlackOccupies);
}

/* Returns if the square is on the board and occupied by the enemy. */
bool isSquareOccupiedByEnemy(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	bool isBlackMoveAndWhiteOccupies = isMovesForBlackPlayer &&
										((board[i][j] == WHITE_K) ||
										 (board[i][j] == WHITE_Q) ||
										 (board[i][j] == WHITE_R) ||
										 (board[i][j] == WHITE_N) ||
										 (board[i][j] == WHITE_B) ||
										 (board[i][j] == WHITE_P));

	bool isWhiteMoveAndBlackOccupies = !isMovesForBlackPlayer &&
										((board[i][j] == BLACK_K) ||
										 (board[i][j] == BLACK_Q) ||
										 (board[i][j] == BLACK_R) ||
										 (board[i][j] == BLACK_N) ||
										 (board[i][j] == BLACK_B) ||
										 (board[i][j] == BLACK_P));

	return (isBlackMoveAndWhiteOccupies || isWhiteMoveAndBlackOccupies);
}

/*
 * Returns the size of the army of the black / white player (according to the input isBlackSoldiers parameter).
 * Results detail how many of each piece remain.
 */
Army getArmy(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackSoldiers)
{
	Army army = { 0 };

	int i, j; // i = column, j = row

	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			char soldier = board[i][j];

			if (isBlackSoldiers)
			{
				switch (soldier)
				{
					case BLACK_P:
					{
						army.pawns++;
						break;
					}
					case BLACK_B:
					{
						army.bishops++;
						break;
					}
					case BLACK_R:
					{
						army.rooks++;
						break;
					}
					case BLACK_N:
					{
						army.knights++;
						break;
					}
					case BLACK_Q:
					{
						army.queens++;
						break;
					}
					case BLACK_K:
					{
						army.kings++;
						break;
					}
					default: { break; }
				}
			}
			else
			{
				switch (soldier)
				{
					case WHITE_P:
					{
						army.pawns++;
						break;
					}
					case WHITE_B:
					{
						army.bishops++;
						break;
					}
					case WHITE_R:
					{
						army.rooks++;
						break;
					}
					case WHITE_N:
					{
						army.knights++;
						break;
					}
					case WHITE_Q:
					{
						army.queens++;
						break;
					}
					case WHITE_K:
					{
						army.kings++;
						break;
					}
					default: { break; }
				}
			}
		}
	}

	return army;
}

/* Validate that there are no pawns in the opponent edge. */
bool validEdges(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i;
	for (i = 0; i < BOARD_SIZE; ++i)
	{
		if (board[i][0] == BLACK_P)
		{	// Bottom edge
			return false;
		}

		if (board[i][7] == WHITE_P)
		{	// Top edge
			return false;
		}
	}

	return true;
}

/* Returns the white / black king's position. */
Position getKingPosition(char board[BOARD_SIZE][BOARD_SIZE], bool isSearchBlackKing)
{
	int i, j; // i = column, j = row

	for (j = 0; j < BOARD_SIZE; j++)
	{
		for (i = 0; i < BOARD_SIZE; i++)
		{
			char soldier = board[j][i];

			if ((isSearchBlackKing && (soldier == BLACK_K)) || (!isSearchBlackKing && (soldier == WHITE_K)))
			{
				Position kingPos = { i, j };
				return kingPos;
			}
		}
	}

	// Invalid position indices, should never reach this line.
	Position invalidKingPos = { INVALID_POSITION_INDEX, INVALID_POSITION_INDEX };
	return invalidKingPos;
}

/* Returns if the square is on the board and occupied by the a pawn of the given color. */
bool isSquareOccupiedByPawn(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_P)) || (!isBlackPiece && (board[i][j] == WHITE_P));
}

/* Returns if the square is on the board and occupied by the a bishop of the given color. */
bool isSquareOccupiedByBishop(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_B)) || (!isBlackPiece && (board[i][j] == WHITE_B));
}

/* Returns if the square is on the board and occupied by the a rook of the given color. */
bool isSquareOccupiedByRook(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_R)) || (!isBlackPiece && (board[i][j] == WHITE_R));
}

/* Returns if the square is on the board and occupied by the a knight of the given color. */
bool isSquareOccupiedByKnight(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_N)) || (!isBlackPiece && (board[i][j] == WHITE_N));
}

/* Returns if the square is on the board and occupied by the a queen of the given color. */
bool isSquareOccupiedByQueen(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_Q)) || (!isBlackPiece && (board[i][j] == WHITE_Q));
}

/* Returns if the square is on the board and occupied by the a king of the given color. */
bool isSquareOccupiedByKing(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPiece, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isBlackPiece && (board[i][j] == BLACK_K)) || (!isBlackPiece && (board[i][j] == WHITE_K));
}

/* Returns if the square is on the edge of the board on the "enemy's side". */
bool isSquareOnOppositeEdge(bool isBlackPiece, int row)
{
	return (!isBlackPiece && (row == (BOARD_SIZE - 1))) || (isBlackPiece && (row == 0));
}

/** Returns true if the square is occupied by a piece of the black player. */
bool isSquareOccupiedByBlackPlayer(char board[BOARD_SIZE][BOARD_SIZE], int x, int y)
{
	return isSquareOccupiedByCurrPlayer(board, true, x, y); //If occupied by black, this is true
}

/** Returns true if the square is occupied by a piece of the white player. */
bool isSquareOccupiedByWhitePlayer(char board[BOARD_SIZE][BOARD_SIZE], int x, int y)
{
	return isSquareOccupiedByCurrPlayer(board, false, x, y); //If occupied by white, this is true
}