#include <stdlib.h>
#include <string.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameCommands.h"
#include "GameLogic.h"
#include "Minimax.h"

/* -- Globals Definition -- */

/* The game mode. There are two possible values: 1 - two players mode, and 2 - player vs. AI mode. Default to 1. */
int g_gameMode = 1;

/* Configuration for mini-max algorithm. Maximum depth of recursion. Default to 1. */
int g_minimaxDepth = 1;

/* Is the user the black color (true) or white color (false). Default to white. */
bool g_isUserBlack = false;

/* Is the next player the black color (true) or white color (false). Default to white. */
bool g_isNextPlayerBlack = false;

/* A buffer allocated for reading user input. The user's command will not exceed 50 characters. */
char g_inputLine[LINE_LENGTH] = { 0 };

/* True if there was an allocation error somewhere in the program (would cause an exit). Else false. */
bool g_memError = false;

/* -- Functions -- */

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
	char xCoord = arg[1];
	char yCoord = arg[3];

	pos.x = xCoord - 'a';

	// Treat the edge case of position "<c,1X>" -- need to parse 2 digits
	if (('1' == yCoord) && ('>' != arg[4]))
		pos.y = 9 + (arg[4] - '0');
	else
		pos.y = yCoord - '1';

	return pos;
}

/* Validate board initialization. If it is valid the program can move to game state. */
bool validStart(char board[BOARD_SIZE][BOARD_SIZE])
{
	// Count white and black soldiers
	Army whiteArmy = getArmy(board, false);
	int totalWhite = whiteArmy.pawns + whiteArmy.bishops + whiteArmy.rooks
					 + whiteArmy.knights + whiteArmy.queens + whiteArmy.kings;
	Army blackArmy = getArmy(board, true);
	int totalBlack = blackArmy.pawns + blackArmy.bishops + blackArmy.rooks
					 + blackArmy.knights + blackArmy.queens + blackArmy.kings;

	if ((totalWhite == 0) || (totalBlack == 0)	// The board is empty or there are pieces of only one color
		|| (whiteArmy.kings == 0) || (blackArmy.kings == 0)	// One of the kings is missing
		|| (totalWhite > MAX_SOLDIERS) || (totalBlack > MAX_SOLDIERS)	// There are more than 16 pieces of the same color
		|| (!validEdges(board)))	// There is a pawn in the opponent edge
	{	// Invalid start
		return false;
	}

	return true;
}

/*
 * Parse next user setting during Settings state and execute it.
 * Return RETRY if the settings haven't done, QUIT if a quit command was entered
 * and SUCCESS if a success command was entered (and the board initialization is valid).
 */
COMMAND_RESULT parseUserSettings(char board[BOARD_SIZE][BOARD_SIZE])
{
	COMMAND_RESULT commandResult = RETRY;
	char* args[MAX_ARGS];
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
			else
			{	// Illegal mode
				printf(WRONG_GAME_MODE);
			}

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
					{
						g_minimaxDepth = depth;
					}
					else
					{	// Illegal depth
						printf(WRONG_MINIMAX_DEPTH);
					}
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
				{
					g_isUserBlack = false;
				}
				else if (0 == strcmp("black", args[1]))
				{
					g_isUserBlack = true;
				}
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
			{
				g_isNextPlayerBlack = false;
			}
			else if (0 == strcmp("black", args[1]))
			{
				g_isNextPlayerBlack = true;
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(REMOVE_COMMAND, args[0]))
		{	// Remove
			Position pos = argToPosition(args[1]);
			// Validate position
			if (isSquareOnBoard(pos.x, pos.y))
			{
				board[pos.x][pos.y] = EMPTY;
			}
			else
			{
				printf(WRONG_POSITION);
			}

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
	char* args[MAX_ARGS];
	int argc = breakInputToArgs(args);
	if (g_memError)
		return QUIT;

	if (argc > 0)
	{
		if (0 == strcmp(MOVE_COMMAND, args[0]))
		{	// Move
			bool isMoveExecuted = executeMoveCommand(board, isUserBlack, args);
			if (g_memError)
				return QUIT;

			if (!isMoveExecuted)	// Illegal command
				commandResult = RETRY;
			else
				commandResult = SUCCESS;
		}
		else if (0 == strcmp(GET_MOVES_COMMAND, args[0]))
		{	// Get Moves
			executeGetMovesCommand(board, isUserBlack, args[1]);
			if (g_memError)
				return QUIT;
			
			commandResult = RETRY;
		}
		else if (0 == strcmp(GET_BEST_MOVES_COMMAND, args[0]))
		{	// Get Best Moves
			executeGetBestMovesCommand(board, isUserBlack, args[1]);
			if (g_memError)
				return QUIT;

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
	// Compute next move by computer using the minimax algorithm
	bool isComputerBlack = !isUserBlack;
	Move* nextMove = minimax(board, isComputerBlack);
	if (g_memError)
		return;

	if (NULL == nextMove)
		return; // Should never happen, but this should protect us from collapsing if it does

	printf(COMPUTER_MSG);
	printMove(nextMove);

	executeMove(board, nextMove);
}

/* 
 * Check for checkmate or a tie and print appropriate message.
 * Return true if it is a termination case (mate or tie) and false else.
 */
bool checkMateTie(char board[BOARD_SIZE][BOARD_SIZE], bool isBlack)
{
	LinkedList* moves = getMoves(board, isBlack);
	if (g_memError)
		return false;

	if (isCheck(board, isBlack))
	{
		if (moves->length == 0)
		{	// Mate
			printf(WIN_MSG, isBlack ? WHITE_STR : BLACK_STR);
			deleteList(moves);
			return true;
		}
		else
		{	// Check
			printf(CHECK);
		}
	}
	else if (moves->length == 0)
	{	// Tie
		printf(TIE);
		deleteList(moves);
		return true;
	}

	deleteList(moves);
	return false;
}

/*
 * Executes the game loop.
 * The two users, or the computer in Player Vs. AI mode, each take turns until one of them wins or the user quits.
 * This function runs an entire single Chess game.
 * Input:
 *		board ~ The initial game board.
 *		isUserBlack ~ Applicable only in Player Vs. AI mode, whether the user is black (true) or white (false).
 */
void executeGameLoop(char board[BOARD_SIZE][BOARD_SIZE], int gameMode, bool isNextPlayerBlack, bool isUserBlack)
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

int todo_main(int argc, char *argv[])
{
	printf(WELCOME_TO_CHESS);

	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	print_board(board);
	printf("\n");

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
		
		executeGameLoop(board, g_gameMode, g_isNextPlayerBlack, g_isUserBlack);
	}
	
	return 0;
}