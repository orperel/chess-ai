#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameCommands.h"
#include "GameLogic.h"
#include "Minimax.h"

/* A "toString()" function for Move structs. */
void printMove(Move* move)
{
	printf("<%c,%d> to <%c,%d>", 'a' + move->initPos.x, move->initPos.y + 1,
		'a' + move->nextPos.x, move->nextPos.y + 1);

	// We assume that the promotion field is valid
	if (move->promotion == EMPTY)
	{
		printf("\n");
	}
	else if ((move->promotion == WHITE_Q) || (move->promotion == BLACK_Q))
	{
		printf(" %s\n", QUEEN);
	}
	else if ((move->promotion == WHITE_B) || (move->promotion == BLACK_B))
	{
		printf(" %s\n", BISHOP);
	}
	else if ((move->promotion == WHITE_R) || (move->promotion == BLACK_R))
	{
		printf(" %s\n", ROOK);
	}
	else if ((move->promotion == WHITE_N) || (move->promotion == BLACK_N))
	{
		printf(" %s\n", KNIGHT);
	}
}

/* Returns if the 2 positions are equal in their content. */
bool isEqualPositions(Position* a, Position* b)
{
	return ((a->x == b->x) && (a->y == b->y));
}

/* Returns if the 2 moves are equal in their content. */
bool isEqualMoves(Move* a, Move* b)
{
	// Check initPos
	if (!isEqualPositions(&a->initPos, &b->initPos))
		return false;

	// Check nextPos
	if (!isEqualPositions(&a->nextPos, &b->nextPos))
		return false;

	// Check promotion
	if (a->promotion != b->promotion)
		return false;

	return true;
}

/* Convert promotion type name to its console representation according to isBlack parameter. */
char promotionNameToChar(char* name, bool isBlack)
{
	if (0 == strcmp(QUEEN, name))
	{
		return ((isBlack) ? BLACK_Q : WHITE_Q);
	}
	else if (0 == strcmp(BISHOP, name))
	{
		return ((isBlack) ? BLACK_B : WHITE_B);
	}
	else if (0 == strcmp(ROOK, name))
	{
		return ((isBlack) ? BLACK_R : WHITE_R);
	}
	else if (0 == strcmp(KNIGHT, name))
	{
		return ((isBlack) ? BLACK_N : WHITE_N);
	}

	return EMPTY;
}

/*
 * Parses and builds "Move" struct out of the arguments.
 * The Move will also be validated. If it is illegal, NULL is returned.
 * If the function succeeds and this is a valid move, the move is returned.
 */
Move* parseAndBuildMove(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* args[])
{
	Position initPos = argToPosition(args[1]);
	Position nextPos = argToPosition(args[3]);

	// Validation #1 - Invalid position
	if (!isSquareOnBoard(initPos.x, initPos.y) || !isSquareOnBoard(nextPos.x, nextPos.y))
	{
		printf(WRONG_POSITION);
		return NULL;
	}

	// Validation #2 - Piece does not belong to player
	if (!isSquareOccupiedByCurrPlayer(board, isUserBlack, initPos.x, initPos.y))
	{
		printf(NO_PIECE);
		return NULL;
	}

	// Build the move
	Move* move = createMove(&initPos, &nextPos);
	if (g_memError)
		return NULL;

	// Update promotion
	if (args[4] != NULL)
	{	// Promotion was specified, we assume that the type name is valid
		if (board[initPos.x][initPos.y] == WHITE_P)
		{
			move->promotion = promotionNameToChar(args[4], false);
		}
		else if (board[initPos.x][initPos.y] == BLACK_P)
		{
			move->promotion = promotionNameToChar(args[4], true);
		}
		else
		{	// Promotion was specified not for a pawn
			printf(ILLEGAL_MOVE);
			deleteMove((void*)move);
			return NULL;
		}
	}
	else
	{	// Promotion was not specified, check for default promotion
		if ((nextPos.y == (BOARD_SIZE - 1)) && (board[nextPos.x][nextPos.y] == WHITE_P))
		{
			move->promotion = WHITE_Q;
		}
		else if ((nextPos.y == 0) && (board[nextPos.x][nextPos.y] == BLACK_P))
		{
			move->promotion = BLACK_Q;
		}
	}

	// Validation #3 - Is the move legal (we compare the move against all legal moves)
	LinkedList* possibleMoves = getMoves(board, isUserBlack);
	if (g_memError)
	{
		deleteMove((void*)move);
		return NULL;
	}

	Node* currMoveNode = possibleMoves->head;
	bool isLegalMove = false;
	while ((!isLegalMove) && (NULL != currMoveNode))
	{
		Move* currMove = (Move*)currMoveNode->data;
		isLegalMove = isEqualMoves(move, currMove);
		currMoveNode = currMoveNode->next;
	}

	deleteList(possibleMoves);	// Free resources

	if (!isLegalMove)
	{ // Validation #3 failed
		printf(ILLEGAL_MOVE);
		deleteMove((void*)move);
		return NULL;
	}

	return move;
}

/*
 * Executes a move command done by the user.
 * This function parses, builds, validates and executes the move.
 * Returns True if the move is legal and executed successfully.
 * If this move is impossible, False is returned.
 */
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* args[MAX_ARGS])
{
	Move* move = parseAndBuildMove(board, isUserBlack, args);

	if (NULL == move) // Illegal move
		return false;

	// Move is valid and now contains all information we need to execute the next step
	if (!executeMove(board, move))
		return false;

	return true;
}

/* Prints a list of all possible moves for the player. */
void executeGetMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* arg)
{
	Position pos = argToPosition(arg);
	// Validation #1 - Invalid position
	if (!isSquareOnBoard(pos.x, pos.y))
	{
		printf(WRONG_POSITION);
		return;
	}
	// Validation #2 - Piece does not belong to player
	if (!isSquareOccupiedByCurrPlayer(board, isUserBlack, pos.x, pos.y))
	{
		printf(NO_PIECE);
		return;
	}

	// Get the moves and print them
	LinkedList* possibleMoves = getMovesForSquare(board, pos.x, pos.y);
	if (g_memError)
		return;

	Node* currMoveNode = possibleMoves->head;
	while (NULL != currMoveNode)
	{
		Move* currMove = (Move*)currMoveNode->data;
		printMove(currMove);
		currMoveNode = currMoveNode->next;
	}

	deleteList(possibleMoves);
}

/*
 * Return the score for the given move in a minimax tree of the given depth.
 * If there was an allocation error return INT_MIN.
 */
int executeGetScoreCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* depth, Move* move)
{
	GameStep* gameStep = createGameStep(board, move);	// Convert Move to gameStep

	if (g_memError)
		return INT_MIN;

	doStep(board, gameStep);

	// Call alphabeta algorithm with the requested depth, on the requested move
	int tempDepth = g_minimaxDepth;
	if (0 != strcmp(DIFFICULTY_BEST, depth))
		g_minimaxDepth = atoi(depth);
	else
		g_minimaxDepth = MAX_DEPTH;	// Difficulty best is implemented as the max depth

	int score = alphabeta(board, 1, INT_MIN, INT_MAX, !isUserBlack);

	g_minimaxDepth = tempDepth;

	undoStep(board, gameStep);
	deleteGameStep(gameStep);

	return score;
}

/*
 * Print all the moves with the highest score for the current board.
 * 'depth' is an argument for the minimax algorithm.
 */
void executeGetBestMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, char* depth)
{
	// Get moves for the current board
	LinkedList* possibleMoves = getMoves(board, isUserBlack);
	if (g_memError)
		return;

	// Compute scores using executeGetScoreCommand and find the max
	int* scores = (int*)malloc(sizeof(int) * possibleMoves->length);
	if (scores == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		deleteList(possibleMoves);
		return;
	}

	Node* currMoveNode = possibleMoves->head;
	int i = 0;
	int maxScore = INT_MIN;
	while (NULL != currMoveNode)
	{
		Move* currMove = (Move*)currMoveNode->data;
		*(scores + i) = executeGetScoreCommand(board, isUserBlack, depth, currMove);
		if (g_memError)
		{
			deleteList(possibleMoves);
			return;
		}

		if (*(scores + i) > maxScore)
		{
			maxScore = *(scores + i);
		}

		currMoveNode = currMoveNode->next;
		i++;
	}

	// Print the moves with the highest score
	currMoveNode = possibleMoves->head;
	i = 0;
	while (NULL != currMoveNode)
	{
		if (*(scores + i) == maxScore)
		{
			Move* currMove = (Move*)currMoveNode->data;
			printMove(currMove);
		}

		currMoveNode = currMoveNode->next;
		i++;
	}

	deleteList(possibleMoves);
	free(scores);
}