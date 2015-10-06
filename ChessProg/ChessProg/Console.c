#include "Console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "BoardManager.h"
#include "GameCommands.h"
#include "LinkedList.h"

/** -- Console constants -- */
// (these constants are private to the console so they are declared here)
#define PAWN "pawn"
#define BISHOP "bishop"
#define ROOK "rook"
#define KNIGHT "knight"
#define QUEEN "queen"
#define KING "king"

#define GAME_MODE_COMMAND "game_mode"
#define DIFFICULTY_COMMAND "difficulty"
#define DIFFICULTY_DEPTH "depth"
#define USER_COLOR_COMMAND "user_color"
#define LOAD_COMMAND "load"
#define CLEAR_COMMAND "clear"
#define NEXT_PLAYER_COMMAND "next_player"
#define REMOVE_COMMAND "rm"
#define SET_COMMAND "set"
#define PRINT_COMMAND "print"
#define QUIT_COMMAND "quit"
#define START_COMMAND "start"
#define MOVE_COMMAND "move"
#define GET_MOVES_COMMAND "get_moves"
#define GET_BEST_MOVES_COMMAND "get_best_moves"
#define GET_SCORE_COMMAND "get_score"
#define SAVE_COMMAND "save"

#define WELCOME_TO_CHESS "Welcome to Chess!\n\n"
#define ENTER_SETTINGS "Enter game settings:\n" 
#define WRONG_GAME_MODE "Wrong game mode\n"
#define TWO_PLAYERS_GAME_MODE "Running game in 2 players mode\n"
#define PLAYER_VS_AI_GAME_MODE "Running game in player vs. AI mode\n"
#define WRONG_BOARD_INITIALIZATION "Wrong board initialization\n"
#define ENTER_YOUR_MOVE "%s player - enter your move:\n"
#define COMPUTER_MSG "Computer: move "

#define ILLEGAL_COMMAND "Illegal command, please try again\n"
#define ILLEGAL_MOVE "Illegal move\n"

#define WRONG_ROOK_POSITION "Wrong position for a rook\n" 
#define ILLEGAL_CASTLING_MOVE "Illegal castling move\n"  

#define CHECK "Check!\n"
#define TIE "The game ends in a tie\n"
#define WIN_MSG "Mate! %s player wins the game\n"

/** -- Logic functions -- */

/* A "toString()" function for Move structs (for console). */
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
		if ((nextPos.x == (BOARD_SIZE - 1)) && (board[initPos.x][initPos.y] == WHITE_P))
		{
			move->promotion = WHITE_Q;
		}
		else if ((nextPos.x == 0) && (board[initPos.x][initPos.y] == BLACK_P))
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
			if (mode == GAME_MODE_2_PLAYERS)
			{	// Two players mode
				g_gameMode = mode;
				printf(TWO_PLAYERS_GAME_MODE);
			}
			else if (mode == GAME_MODE_PLAYER_VS_AI)
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
			if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
			{
				if (0 == strcmp(DIFFICULTY_DEPTH, args[1]))
				{
					int depth = atoi(args[2]);
					if ((depth >= 1) && (depth <= MAX_DEPTH))
					{
						g_minimaxDepth = depth;
						g_isDifficultyBest = false;
					}
					else	// Illegal depth
						printf(WRONG_MINIMAX_DEPTH);
				}
				else if (0 == strcmp(DIFFICULTY_BEST, args[1]))
				{	// Set difficulty best to MAX_DEPTH
					g_minimaxDepth = MAX_DEPTH;
					g_isDifficultyBest = true;
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
			if (g_gameMode == GAME_MODE_PLAYER_VS_AI)
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
			if (isValidStart(board))
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
 * Execute Setting state (determine settings).
 * "board" is the initial game board.
 * Return true if START_COMMAND was entered; false if QUIT_COMMAND was entered.
 */
bool determineGameSettings(char board[BOARD_SIZE][BOARD_SIZE])
{
	COMMAND_RESULT command = RETRY;

	// Get settings input loop
	while (RETRY == command)
	{
		printf(ENTER_SETTINGS);

		getUserInput();

		command = parseUserSettings(board);
	}

	if (QUIT == command)
	{
		return false;
	}

	return true;
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
			int depth;
			if (0 == strcmp(args[1], DIFFICULTY_BEST))
				depth = DIFFICULTY_BEST_INT;
			else
				depth = atoi(args[1]);

			LinkedList* bestMoves = executeGetBestMovesCommand(board, isUserBlack, depth);
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
				int depth;
				if (0 == strcmp(args[1], DIFFICULTY_BEST))
					depth = DIFFICULTY_BEST_INT;
				else
					depth = atoi(args[1]);

				int score = executeGetScoreCommand(board, isUserBlack, depth, move);
				if (g_memError)
					return QUIT;

				printf("%d\n", score);

				deleteMove((void*)move);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(SAVE_COMMAND, args[0]))
		{	// Save
			executeSaveCommand(board, args[1], isUserBlack);

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

/*
* Executes the next turn done by the user.
* This function will repeat until the user executes a legal move.
* Return true if the user quits the game. False if the game continues.
*/
bool executeUserTurn(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	COMMAND_RESULT command = RETRY;

	while (RETRY == command)
	{
		printf(ENTER_YOUR_MOVE, isUserBlack ? BLACK_STR : WHITE_STR);

		getUserInput();

		command = parseUserCommand(board, isUserBlack);
	}

	return (QUIT == command);
}

/* Executes the next turn done by the computer. */
void executeComputerTurn(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	Move* nextMove = executeGetNextComputerMoveCommand(board, isUserBlack);
	if (NULL == nextMove)
		return;

	printf(COMPUTER_MSG);
	printMove(nextMove);

	executeMove(board, nextMove);
}

/*
* Check for checkmate or a tie and return the state of the board.
* Note: mate or tie are termination cases.
*/
bool checkMateTie(char board[BOARD_SIZE][BOARD_SIZE], bool isBlack)
{
	ChessGameState state = executeCheckMateTieCommand(board, isBlack);
	bool isTerminate = false;

	switch (state)
	{
	case(GAME_MATE_BLACK_WINS) :
	{
		printf(WIN_MSG, BLACK_STR);
		isTerminate = true;
		break;
	}
	case(GAME_MATE_WHITE_WINS) :
	{
		printf(WIN_MSG, WHITE_STR);
		isTerminate = true;
		break;
	}
	case(GAME_CHECK) :
	{
		printf(CHECK);
		isTerminate = false;
		break;
	}
	case(GAME_TIE) :
	{
		printf(TIE);
		isTerminate = true;
		break;
	}
	case(GAME_ERROR) :
	{
		isTerminate = true;
		break;
	}
	default:
	{
		isTerminate = false;
	}
	}

	return isTerminate;
}

/*
 * Executes the console game loop.
 * The two users, or the computer in Player Vs. AI mode, each take turns until one of them wins or the user quits.
 * This function runs an entire single Chess game.
 * Input:
 *		board ~ The initial game board.
 *		isUserBlack ~ Applicable only in Player Vs. AI mode, whether the user is black (true) or white (false).
 */
void executeConsoleGameLoop(char board[BOARD_SIZE][BOARD_SIZE], int gameMode, bool isNextPlayerBlack, bool isUserBlack)
{
	bool isUserTurn = true;	// By default the game mode is Two Players mode, so it is always the user turn
	if (gameMode == 2)
	{	// Player Vs. AI mode
		isUserTurn = (isUserBlack && isNextPlayerBlack) || (!isUserBlack && !isNextPlayerBlack);
	}
	bool isBlackTurn = isNextPlayerBlack;
	bool isQuit = false;

	// Game loops while the user doesn't quit and there's no victor
	while (!isQuit)
	{
		// Game continues, execute next turn
		if (isUserTurn)
		{
			isQuit = executeUserTurn(board, isBlackTurn);
		}
		else
		{
			executeComputerTurn(board, isUserBlack);
		}

		if (g_memError)
			return;

		if (!isQuit) // If the user doesn't quit, we advance the game loop
		{
			print_board(board);

			// Check for victory or tie - to see if game ends
			isQuit = checkMateTie(board, !isBlackTurn);
			if (g_memError)
				return;

			// Change turns. In Two Players mode we don't change the flag isUserTurn
			if (gameMode == 2)
			{
				isUserTurn = !isUserTurn;
			}
			isBlackTurn = !isBlackTurn;
		}
	}
}

/* ##### To remove before submission ##### */
void computerVsComputer(char board[BOARD_SIZE][BOARD_SIZE])
{
	bool isQuit = false;
	while (!isQuit)
	{
		if (!g_isNextPlayerBlack)
			g_minimaxDepth = 3;	// White turn 
		else
			g_minimaxDepth = 1;	// Black turn

		executeComputerTurn(board, !g_isNextPlayerBlack);	// executeComputerTurn switches the color
		if (g_memError)
			return;

		printf("%d\n", g_boardsCounter);

		if (!isQuit)
		{
			print_board(board);

			isQuit = checkMateTie(board, !g_isNextPlayerBlack);
			if (g_memError)
				return;

			g_isNextPlayerBlack = !g_isNextPlayerBlack;
		}
	}
}

/*
 * Initiates the console game loop.
 * The two users, or the computer in Player Vs. AI mode, each take turns until one of them wins or the user quits.
 * This function runs an entire single Chess game, after initializing the board and determining game settings.
 */
int initConsoleMainLoop()
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	print_board(board);

	// Start game settings mode.
	// If the user doesn't quit, we start the game (determineGameSettings returns true).
	if (determineGameSettings(board))
	{
		// Treat the edge case of a game board where one player immediately loses due to a non-fair game setting.
		// White wins (or tie)
		bool stuckResult = checkMateTie(board, true);
		if (g_memError)
			return -1;
		if (stuckResult)
			return 0;

		// Black wins (or tie)
		stuckResult = checkMateTie(board, false);
		if (g_memError)
			return -1;
		if (stuckResult)
			return 0;

		executeConsoleGameLoop(board, g_gameMode, g_isNextPlayerBlack, g_isUserBlack);
	}

	return 0;
}