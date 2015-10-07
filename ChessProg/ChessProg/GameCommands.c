#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "BoardManager.h"
#include "GameCommands.h"
#include "GameLogic.h"
#include "Minimax.h"

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

/** Returns true if the move is a legal move by the given player (black or white).
 *  Validation is done by comparing the move to all legal moves, so make sure to query the mem flag on return.
 */
bool validateMove(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, Move* move)
{
	// Validation #3 - Is the move legal (we compare the move against all legal moves)
	LinkedList* possibleMoves = getMoves(board, isUserBlack);
	if (g_memError)
	{
		return false;
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

	return isLegalMove;
}

/*
 * Executes a move command done by the user.
 * Move is expected to be validated with validateMove() before calling this function.
 * This function safely executes the move.
 * Returns True if the move is legal and executed successfully.
 * If this move failed to execute (due to an unexpected memory error), False is returned.
 */
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], Move* move)
{
	if (NULL == move) // Illegal move
		return false;

	// Move is valid and now contains all information we need to execute the next step
	if (!executeMove(board, move))
		return false;

	return true;
}

/* Returns a list of all possible moves for the player for one square.
 * List must be freed by user when usage terminates.
 */
LinkedList* executeGetMovesForPosCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, Position pos)
{
	// Validation #1 - Invalid position (note: this state is invalid in gui mode)
	if (!isSquareOnBoard(pos.x, pos.y))
	{
		printf(WRONG_POSITION);
		return NULL;
	}
	// Validation #2 - Piece does not belong to player (note: this state is invalid in gui mode)
	if (!isSquareOccupiedByCurrPlayer(board, isUserBlack, pos.x, pos.y))
	{
		printf(NO_PIECE);
		return NULL;
	}

	// Get the moves and print them
	LinkedList* possibleMoves = getMovesForSquare(board, pos.x, pos.y);
	
	return possibleMoves;
}

/*
 * Return the score for the given move in a minimax tree of the given depth.
 * If there was an allocation error return INT_MIN.
 */
int executeGetScoreCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, int depth, Move* move)
{
	GameStep* gameStep = createGameStep(board, move);	// Convert Move to gameStep

	if (g_memError)
		return INT_MIN;

	doStep(board, gameStep);
	g_boardsCounter++;

	// Call alphabeta algorithm with the requested depth, on the requested move
	int tempDepth = g_minimaxDepth;
	if (depth == DIFFICULTY_BEST_INT)
		g_minimaxDepth = MAX_DEPTH;	// Difficulty best is implemented as the max depth
	else
		g_minimaxDepth = depth;

	int score = alphabeta(board, 1, INT_MIN, INT_MAX, !isUserBlack);

	g_minimaxDepth = tempDepth;

	undoStep(board, gameStep);
	deleteGameStep(gameStep);

	return score;
}

/*
 * Return all the moves with the highest score for the current board.
 * 'depth' is an argument for the minimax algorithm.
 * List of moves must be freed when usage is complete.
 */
LinkedList* executeGetBestMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack, int depth)
{
	g_boardsCounter = 0;
	// Get moves for the current board
	LinkedList* possibleMoves = getMoves(board, isUserBlack);
	if (g_memError)
		return NULL;

	// Compute scores using executeGetScoreCommand and find the max
	int* scores = (int*)malloc(sizeof(int) * possibleMoves->length);
	if (scores == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		deleteList(possibleMoves);
		return NULL;
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
			free(scores);
			return NULL;
		}

		if (*(scores + i) > maxScore)
		{
			maxScore = *(scores + i);
		}

		currMoveNode = currMoveNode->next;
		i++;
	}

	// Collect the moves with the highest score
	LinkedList* bestMoves = createList(deleteMove);
	if (g_memError)
	{
		deleteList(possibleMoves);
		free(scores);
		return NULL;
	}

	currMoveNode = possibleMoves->head;
	i = 0;
	while (NULL != currMoveNode)
	{
		if (*(scores + i) == maxScore)
		{
			Move* currMove = cloneMove((Move*)currMoveNode->data);
			insertLast(bestMoves, currMove);
			if (g_memError)
			{
				deleteList(possibleMoves);
				free(scores);
				deleteList(bestMoves);
				return NULL;
			}
		}

		currMoveNode = currMoveNode->next;
		i++;
	}

	deleteList(possibleMoves);
	free(scores);

	return bestMoves;
}

/* Fetch the next turn done by the computer. */
Move* executeGetNextComputerMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	// Compute next move by computer using the minimax algorithm
	bool isComputerBlack = !isUserBlack;
	Move* nextMove = minimax(board, isComputerBlack);
	if (g_memError)
		return NULL;

	return nextMove;
}

/*
 * Check for checkmate or a tie and return the state of the board.
 * Note: mate or tie are termination cases.
 */
ChessGameState executeCheckMateTieCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isBlack)
{
	LinkedList* moves = getMoves(board, isBlack);
	if (g_memError)
		return GAME_ERROR;

	ChessGameState state = GAME_ONGOING;

	if (isCheck(board, isBlack))
	{
		if (moves->length == 0)
		{	// Mate
			state = isBlack ? GAME_MATE_WHITE_WINS : GAME_MATE_BLACK_WINS;
		}
		else
		{	// Check
			state = GAME_CHECK;
		}
	}
	else if (moves->length == 0)
	{	// Tie
		state = GAME_TIE;
	}

	deleteList(moves);
	return state;
}

/*
 * Load the game settings from the file "path", path being the full or relative path to the file.
 * We assume that the file contains valid data and is correctly formatted.
 * Return True if the loading ended successfully, else False.
 */
bool executeLoadCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path)
{
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf(WRONG_FILE_NAME);
		return false;
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
				return false;
			}
		}
		else if (strcmp(token, GAME_MODE_TAG_BEGIN) == 0)
		{	// Game mode tag
			// Get the tag content
			token = strtok(NULL, ">");
			token = strtok(token, "<");

			if (token[0] == '1')
				g_gameMode = GAME_MODE_2_PLAYERS;
			else if (token[0] == '2')
				g_gameMode = GAME_MODE_PLAYER_VS_AI;
			else
			{	// Should never reach this code
				printf(WRONG_FORMAT);
				fclose(fp);
				return false;
			}
		}
		else if (strcmp(token, DIFFICULTY_TAG_BEGIN) == 0)
		{	// Difficulty tag
			if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
			{	// Difficulty is applicable only in player vs. AI mode
				token = strtok(NULL, ">");
				token = strtok(token, "<");

				if (strcmp(token, DIFFICULTY_BEST) == 0)
				{	// Set difficulty best to MAX_DEPTH
					g_minimaxDepth = MAX_DEPTH;
					g_isDifficultyBest = true;
				}
				else
				{
					g_minimaxDepth = atoi(token);
					g_isDifficultyBest = false;
				}
			}
		}
		else if (strcmp(token, USER_COLOR_TAG_BEGIN) == 0)
		{	// User color tag
			if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
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
					return false;
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
			return false;
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
	
	return true;
}

/*
 * Save the current game state to the file "path".
 * Return True if the saving ended successfully, else False.
 */
bool executeSaveCommand(char board[BOARD_SIZE][BOARD_SIZE], char* path, bool isBlackTurn)
{
	FILE* fp = fopen(path, "w");
	if (fp == NULL)
	{
		printf(WRONG_FILE_NAME);
		return false;
	}

	// Write the header and the game tag begin
	fprintf(fp, "%s\n%s\n", XML_HEADER, GAME_TAG_BEGIN);

	// Write the next turn
	fprintf(fp, "\t%s>", NEXT_TURN_TAG_BEGIN);
	if (isBlackTurn)
		fprintf(fp, "%s%s\n", WHITE_STR, NEXT_TURN_TAG_END);	// The next player is white
	else
		fprintf(fp, "%s%s\n", BLACK_STR, NEXT_TURN_TAG_END);	// The next player is black

	// Write the game mode
	fprintf(fp, "\t%s>%d%s\n", GAME_MODE_TAG_BEGIN, g_gameMode, GAME_MODE_TAG_END);

	// Write the difficulty
	fprintf(fp, "\t%s>", DIFFICULTY_TAG_BEGIN);
	if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
	{
		if (g_isDifficultyBest)
			fprintf(fp, "%s", DIFFICULTY_BEST);
		else
			fprintf(fp, "%d", g_minimaxDepth);
	}
	fprintf(fp, "%s\n", DIFFICULTY_TAG_END);

	// Write the user color
	fprintf(fp, "\t%s>", USER_COLOR_TAG_BEGIN);
	if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
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

	return true;
}

/* Validate board initialization. If it is valid the program can move to game state. */
bool isValidStart(char board[BOARD_SIZE][BOARD_SIZE])
{
	// Count white and black soldiers
	Army whiteArmy = getArmy(board, false);
	Army blackArmy = getArmy(board, true);

	if ((whiteArmy.kings == 0) || (blackArmy.kings == 0)	// One of the kings is missing
		|| (whiteArmy.kings > 1) || (blackArmy.kings > 1)	// There are more than one king
		|| (whiteArmy.queens > 1) || (blackArmy.queens > 1)	// There are more than one queen
		|| (whiteArmy.knights > 2) || (blackArmy.knights > 2)	// There are more than two knights
		|| (whiteArmy.rooks > 2) || (blackArmy.rooks > 2)	// There are more than two rooks
		|| (whiteArmy.bishops > 2) || (blackArmy.bishops > 2)	// There are more than two bishops
		|| (whiteArmy.pawns > 8) || (blackArmy.pawns > 8)	// There are more than eight pawns
		|| (!validEdges(board)))	// There is a pawn in the opponent edge
	{	// Invalid start
		return false;
	}

	return true;
}