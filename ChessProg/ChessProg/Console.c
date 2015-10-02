#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameCommands.h"
#include "Console.h"

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

/** Prints the list of moves in a formatted way (list is expected to contain only Move* types). */
void printListOfMoves(LinkedList* moves)
{
	if (moves == NULL)
		return;

	Node* currMoveNode = moves->head;
	while (NULL != currMoveNode)
	{
		Move* currMove = (Move*)currMoveNode->data;
		printMove(currMove);
		currMoveNode = currMoveNode->next;
	}
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
	if (!validateMove(board, isUserBlack, move))
	{ // Validation #3 failed
		if (!g_memError)
			printf(ILLEGAL_MOVE); // False may return on mem errors too so query the global mem error flag

		deleteMove((void*)move);
		return NULL;
	}

	return move;
}

/* Reads a user input line into g_inputLine as null terminated string. */
void getUserInput()
{
	int ch, i = 0;
	while ((ch = fgetc(stdin)) != '\n')
	{
		g_inputLine[i++] = ch;
	}

	g_inputLine[i] = '\0';
}

/*
 * Breaks the g_inputLine into arguments using space as a delimiter.
 * The results will be packed inside args.
 * Calling functions should make sure to free the memory held by the args when done!
 */
int breakInputToArgs(char* args[MAX_ARGS])
{
	int argc = 0;
	char* argStart = g_inputLine;
	char* nextChar;
	const char nullStr[1] = "";

	nextChar = g_inputLine;
	int argSize = 0;

	// Parse each argument until we reach the maximum amount supported
	while ((argc < MAX_ARGS) && (*nextChar != '\0'))
	{
		// Iterate until space or Null terminate char
		while ((*nextChar != ' ') && (*nextChar != '\0'))
		{
			nextChar++;
			argSize++;
		}

		if (argc == (MAX_ARGS - 1)) // Last arg contains all that remains
			argSize = strlen(argStart);

		char* nextArg = (char*)malloc(sizeof(char) * argSize + 1);

		if (nextArg == NULL)
		{ // Check allocation succeeded
			perror("Error: standard function malloc has failed");
			g_memError = true;
			return -1;
		}

		memcpy(nextArg, argStart, argSize);
		memcpy(nextArg + argSize, nullStr, 1);

		args[argc] = nextArg;
		if (*nextChar != '\0')
			nextChar++;
		argc++;
		argSize = 0;
		argStart = nextChar;
	}

	return argc;
}

/*
 * Returns the position represented by a <i,j> string tuple.
 * This function also converts from the chess logical representation (letter, digit) to array indices.
 */
Position argToPosition(char* arg)
{
	Position pos;
	char xCoord = arg[3];
	char yCoord = arg[1];

	pos.y = yCoord - 'a';

	// Treat the edge case of position "<c,1X>" -- need to parse 2 digits
	if (('1' == xCoord) && ('>' != arg[4]))
		pos.x = 9 + (arg[4] - '0');
	else
		pos.x = xCoord - '1';

	return pos;
}

/*
 * Parse next user setting during Settings state and execute it.
 * Return RETRY if the settings haven't done, QUIT if a quit command was entered
 * and SUCCESS if a success command was entered (and the board initialization is valid).
 */
COMMAND_RESULT parseUserSettings(char board[BOARD_SIZE][BOARD_SIZE])
{
	COMMAND_RESULT commandResult = RETRY;
	char* args[MAX_ARGS] = { 0 };
	int argc = breakInputToArgs(args);
	if (g_memError)
		return QUIT;

	if (argc > 0)
	{
		if (0 == strcmp(GAME_MODE_COMMAND, args[0]))
		{	// Game mode
			int mode = atoi(args[1]);
			if (mode == 1)
			{	// Two players mode
				g_gameMode = mode;
				printf(TWO_PLAYERS_GAME_MODE);
			}
			else if (mode == 2)
			{	// Player vs. AI mode
				g_gameMode = mode;
				printf(PLAYER_VS_AI_GAME_MODE);
			}
			else	// Illegal mode
				printf(WRONG_GAME_MODE);

			commandResult = RETRY;
		}
		else if (0 == strcmp(DIFFICULTY_COMMAND, args[0]))
		{	// Minimax depth
			if (g_gameMode == 2)
			{
				if (0 == strcmp(DIFFICULTY_DEPTH, args[1]))
				{
					int depth = atoi(args[2]);
					if ((depth >= 1) && (depth <= MAX_DEPTH))
						g_minimaxDepth = depth;
					else	// Illegal depth
						printf(WRONG_MINIMAX_DEPTH);
				}
				else if (0 == strcmp(DIFFICULTY_BEST, args[1]))
				{	// Set difficulty best to MAX_DEPTH
					g_minimaxDepth = MAX_DEPTH;
				}
			}
			else
			{
				printf(ILLEGAL_COMMAND);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(USER_COLOR_COMMAND, args[0]))
		{	// User color
			if (g_gameMode == 2)
			{
				if (0 == strcmp("white", args[1]))
					g_isUserBlack = false;
				else if (0 == strcmp("black", args[1]))
					g_isUserBlack = true;
			}
			else
			{
				printf(ILLEGAL_COMMAND);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(LOAD_COMMAND, args[0]))
		{	// Load
			executeLoadCommand(board, args[1]);

			commandResult = RETRY;
		}
		else if (0 == strcmp(CLEAR_COMMAND, args[0]))
		{	// Clear
			clearBoard(board);

			commandResult = RETRY;
		}
		else if (0 == strcmp(NEXT_PLAYER_COMMAND, args[0]))
		{	// Next player
			if (0 == strcmp("white", args[1]))
				g_isNextPlayerBlack = false;
			else if (0 == strcmp("black", args[1]))
				g_isNextPlayerBlack = true;

			commandResult = RETRY;
		}
		else if (0 == strcmp(REMOVE_COMMAND, args[0]))
		{	// Remove
			Position pos = argToPosition(args[1]);
			// Validate position
			if (isSquareOnBoard(pos.x, pos.y))
				board[pos.x][pos.y] = EMPTY;
			else
				printf(WRONG_POSITION);

			commandResult = RETRY;
		}
		else if (0 == strcmp(SET_COMMAND, args[0]))
		{	// Set
			Position pos = argToPosition(args[1]);
			// Validate position
			if (isSquareOnBoard(pos.x, pos.y))
			{
				if (0 == strcmp("white", args[2]))
				{	// Set white
					Army whiteArmy = getArmy(board, false);

					if (0 == strcmp(PAWN, args[3]))
					{	// Pawn
						if (whiteArmy.pawns < 8)
							board[pos.x][pos.y] = WHITE_P;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(BISHOP, args[3]))
					{	// Bishop
						if (whiteArmy.bishops < 2)
							board[pos.x][pos.y] = WHITE_B;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(ROOK, args[3]))
					{	// Rook
						if (whiteArmy.rooks < 2)
							board[pos.x][pos.y] = WHITE_R;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(KNIGHT, args[3]))
					{	// Knight
						if (whiteArmy.knights < 2)
							board[pos.x][pos.y] = WHITE_N;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(QUEEN, args[3]))
					{	// Queen
						if (whiteArmy.queens < 1)
							board[pos.x][pos.y] = WHITE_Q;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(KING, args[3]))
					{	// King
						if (whiteArmy.kings < 1)
							board[pos.x][pos.y] = WHITE_K;
						else
							printf(WRONG_SET);
					}
				}
				else if (0 == strcmp("black", args[2]))
				{	// Set black
					Army blackArmy = getArmy(board, true);

					if (0 == strcmp(PAWN, args[3]))
					{	// Pawn
						if (blackArmy.pawns < 8)
							board[pos.x][pos.y] = BLACK_P;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(BISHOP, args[3]))
					{	// Bishop
						if (blackArmy.bishops < 2)
							board[pos.x][pos.y] = BLACK_B;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(ROOK, args[3]))
					{	// Rook
						if (blackArmy.rooks < 2)
							board[pos.x][pos.y] = BLACK_R;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(KNIGHT, args[3]))
					{	// Knight
						if (blackArmy.knights < 2)
							board[pos.x][pos.y] = BLACK_N;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(QUEEN, args[3]))
					{	// Queen
						if (blackArmy.queens < 1)
							board[pos.x][pos.y] = BLACK_Q;
						else
							printf(WRONG_SET);
					}
					else if (0 == strcmp(KING, args[3]))
					{	// King
						if (blackArmy.kings < 1)
							board[pos.x][pos.y] = BLACK_K;
						else
							printf(WRONG_SET);
					}
				}
			}
			else
			{
				printf(WRONG_POSITION);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(PRINT_COMMAND, args[0]))
		{	// Print
			print_board(board);

			commandResult = RETRY;
		}
		else if (0 == strcmp(QUIT_COMMAND, args[0]))
		{	// Quit
			commandResult = QUIT;
		}
		else if (0 == strcmp(START_COMMAND, args[0]))
		{	// Start
			if (validStart(board))
			{	// The program moves to game state
				commandResult = SUCCESS;
			}
			else
			{
				printf(WRONG_BOARD_INITIALIZATION);
				commandResult = RETRY;
			}
		}
		else
		{	// Illegal command
			printf(ILLEGAL_COMMAND);
			commandResult = RETRY;
		}
	}
	else
	{	// Illegal command
		printf(ILLEGAL_COMMAND);
		commandResult = RETRY;
	}

	// Free args
	int i;
	for (i = 0; i < argc; i++)
		free(args[i]);

	return commandResult;
}

/*
 * Parse next user command during Game state and execute it.
 * Return RETRY if the turn hasn't done yet, QUIT if a quit command was entered
 * and SUCCESS if the command was executed successfully.
 */
COMMAND_RESULT parseUserCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	if (NULL == g_inputLine) // Avoid illegal NULL input
	{
		printf(ILLEGAL_COMMAND);
		return RETRY;
	}

	COMMAND_RESULT commandResult = RETRY;
	char* args[MAX_ARGS] = { 0 };
	int argc = breakInputToArgs(args);
	if (g_memError)
		return QUIT;

	if (argc > 0)
	{
		if (0 == strcmp(MOVE_COMMAND, args[0]))
		{	// Move
			Move* move = parseAndBuildMove(board, isUserBlack, args);
			bool isMoveExecuted = executeMoveCommand(board, isUserBlack, move);
			if (g_memError)
				return QUIT;

			if (!isMoveExecuted)	// Illegal command
				commandResult = RETRY;
			else
				commandResult = SUCCESS;
		}
		else if (0 == strcmp(GET_MOVES_COMMAND, args[0]))
		{	// Get Moves

			Position pos = argToPosition(args[1]);
			LinkedList* possibleMoves = executeGetMovesForPosCommand(board, isUserBlack, pos);
			if (g_memError)
				return QUIT;

			printListOfMoves(possibleMoves);
			deleteList(possibleMoves);
			commandResult = RETRY;
		}
		else if (0 == strcmp(GET_BEST_MOVES_COMMAND, args[0]))
		{	// Get Best Moves
			LinkedList* bestMoves = executeGetBestMovesCommand(board, isUserBlack, args[1]);
			if (g_memError)
				return QUIT;

			printListOfMoves(bestMoves);
			deleteList(bestMoves);
			commandResult = RETRY;
		}
		else if (0 == strcmp(GET_SCORE_COMMAND, args[0]))
		{	// Get Score
			Move* move = parseAndBuildMove(board, isUserBlack, &(args[2]));	// The move location starts in args[2]
			if (g_memError)
				return QUIT;

			if (move != NULL)
			{
				int score = executeGetScoreCommand(board, isUserBlack, args[1], move);
				if (g_memError)
					return QUIT;

				printf("%d\n", score);

				deleteMove((void*)move);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(SAVE_COMMAND, args[0]))
		{	// Save
			executeSaveCommand(board, args[1]);

			commandResult = RETRY;
		}
		else if (0 == strcmp(QUIT_COMMAND, args[0]))
		{	// Quit
			commandResult = QUIT;
		}
		else
		{	// Illegal command
			printf(ILLEGAL_COMMAND);
			commandResult = RETRY;
		}
	}
	else
	{	// Illegal command
		printf(ILLEGAL_COMMAND);
		commandResult = RETRY;
	}

	// Free args
	int i;
	for (i = 0; i < argc; i++)
		free(args[i]);

	return commandResult;
}