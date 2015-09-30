#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameCommands.h"
#include "GameLogic.h"
#include "Minimax.h"
#include "Console.h"

/* A "toString()" function for Move structs. */
void printMove(Move* move)
{
	printf("<%c,%d> to <%c,%d>", 'a' + move->initPos.y, move->initPos.x + 1,
		   'a' + move->nextPos.y, move->nextPos.x + 1);

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
			move->promotion = promotionNameToChar(args[5], false);
		}
		else if (board[initPos.x][initPos.y] == BLACK_P)
		{
			move->promotion = promotionNameToChar(args[5], true);
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
		if ((nextPos.x == (BOARD_SIZE - 1)) && (board[nextPos.x][nextPos.y] == WHITE_P))
		{
			move->promotion = WHITE_Q;
		}
		else if ((nextPos.x == 0) && (board[nextPos.x][nextPos.y] == BLACK_P))
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

/*
 * Load the game settings from the file "path", path being the full or relative path to the file.
 * We assume that the file contains valid data and is correctly formatted.
 */
void executeLoadCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path)
{
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf(WRONG_FILE_NAME);
		return;
	}

	char str[TAG_LENGTH];	// A string for reading the file
	bool done = false;

	// Read until the game tag
	while ((!done) && (fscanf(fp, "%s", str) == 1))
	{
		if (strcmp(GAME_TAG_BEGIN, str) == 0)
			done = true;
	}

	// Read the settings
	done = false;
	char* token;
	while ((!done) && (fscanf(fp, "%s", str) == 1))
	{
		token = strtok(str, ">");
		if (strcmp(token, NEXT_TURN_TAG_BEGIN) == 0)
		{	// Next turn tag
			// Get the tag content
			token = strtok(NULL, ">");
			token = strtok(token, "<");

			if (strcmp(token, WHITE_STR) == 0)
				g_isNextPlayerBlack = false;
			else if (strcmp(token, BLACK_STR) == 0)
				g_isNextPlayerBlack = true;
			else
			{	// Should never reach this code
				printf(WRONG_FORMAT);
				fclose(fp);
				return;
			}
		}
		else if (strcmp(token, GAME_MODE_TAG_BEGIN) == 0)
		{	// Game mode tag
			// Get the tag content
			token = strtok(NULL, ">");
			token = strtok(token, "<");

			if (token[0] == '1')
				g_gameMode = 1;
			else if (token[0] == '2')
				g_gameMode = 2;
			else
			{	// Should never reach this code
				printf(WRONG_FORMAT);
				fclose(fp);
				return;
			}
		}
		else if (strcmp(token, DIFFICULTY_TAG_BEGIN) == 0)
		{	// Difficulty tag
			if (g_gameMode == 2)
			{	// Difficulty is applicable only in player vs. AI mode
				token = strtok(NULL, ">");
				token = strtok(token, "<");

				if (strcmp(token, DIFFICULTY_BEST) == 0)
					g_minimaxDepth = MAX_DEPTH;	// Set difficulty best to MAX_DEPTH
				else
					g_minimaxDepth = atoi(token);
			}
		}
		else if (strcmp(token, USER_COLOR_TAG_BEGIN) == 0)
		{	// User color tag
			if (g_gameMode == 2)
			{	// User color is applicable only in player vs. AI mode
				token = strtok(NULL, ">");
				token = strtok(token, "<");

				if (strcmp(token, WHITE_STR) == 0)
					g_isUserBlack = false;
				else if (strcmp(token, BLACK_STR) == 0)
					g_isUserBlack = true;
				else
				{	// Should never reach this code
					printf(WRONG_FORMAT);
					fclose(fp);
					return;
				}
			}
		}
		else if (strcmp(token, BOARD_TAG_BEGIN) == 0)
		{	// Board tag
			done = true;
		}
		else
		{	// Should never reach this code
			printf(WRONG_FORMAT);
			fclose(fp);
			return;
		}
	}

	// Read the board
	int row = BOARD_SIZE;
	int j;
	while ((row > 0) && (fscanf(fp, "%s", str) == 1))
	{
		token = strtok(str, ">");
		token = strtok(NULL, ">");
		token = strtok(token, "<");

		for (j = 0; j < BOARD_SIZE; j++)
		{
			if (token[j] == '_')
				board[row - 1][j] = EMPTY;
			else
				board[row - 1][j] = token[j];
		}

		row--;
	}

	fclose(fp);
	print_board(board);
}

/* Save the current game state to the file "path". */
void executeSaveCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path)
{
	FILE* fp = fopen(path, "w");
	if (fp == NULL)
	{
		printf(WRONG_FILE_NAME);
		return;
	}

	// Write the header and the game tag begin
	fprintf(fp, "%s\n%s\n", XML_HEADER, GAME_TAG_BEGIN);

	// Write the next turn
	fprintf(fp, "\t%s>", NEXT_TURN_TAG_BEGIN);
	if (g_isNextPlayerBlack)
		fprintf(fp, "%s%s\n", BLACK_STR, NEXT_TURN_TAG_END);
	else
		fprintf(fp, "%s%s\n", WHITE_STR, NEXT_TURN_TAG_END);

	// Write the game mode
	fprintf(fp, "\t%s>%d%s\n", GAME_MODE_TAG_BEGIN, g_gameMode, GAME_MODE_TAG_END);

	// Write the difficulty
	fprintf(fp, "\t%s>", DIFFICULTY_TAG_BEGIN);
	if (g_gameMode == 2)
		fprintf(fp, "%d", g_minimaxDepth);
	fprintf(fp, "%s\n", DIFFICULTY_TAG_END);

	// Write the user color
	fprintf(fp, "\t%s>", USER_COLOR_TAG_BEGIN);
	if (g_gameMode == 2)
	{
		if (g_isUserBlack)
			fprintf(fp, BLACK_STR);
		else
			fprintf(fp, WHITE_STR);
	}	
	fprintf(fp, "%s\n", USER_COLOR_TAG_END);

	// Write the board
	fprintf(fp, "\t%s>\n", BOARD_TAG_BEGIN);
	int row, j;
	for (row = 8; row > 0; row--)
	{
		fprintf(fp, "\t\t%s%d>", ROW_TAG_BEGIN, row);
		for (j = 0; j < BOARD_SIZE; j++)
		{
			if (board[row - 1][j] == EMPTY)
				fprintf(fp, "_");
			else
				fprintf(fp, "%c", board[row - 1][j]);
		}
		fprintf(fp, "%s%d>\n", ROW_TAG_END, row);
	}
	fprintf(fp, "\t%s\n", BOARD_TAG_END);

	// Write the game tag end
	fprintf(fp, "%s\n", GAME_TAG_END);

	fclose(fp);
}