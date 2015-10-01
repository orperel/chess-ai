#include <stdio.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameCommands.h"
#include "GameLogic.h"
#include "Minimax.h"
#include "Console.h"
#include "GuiFW.h"

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

int main(int argc, char *argv[])
{
	//printf(WELCOME_TO_CHESS);
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
	
	getchar();
	return 0;
}