#include <stdlib.h>
#include <string.h>
#include "Types.h"
#include "Chess.h"
#include "BoardManager.h"
#include "GameLogic.h"
#include "Minimax.h"

/* -- Globals Definition -- */

/* Configuration for mini-max algorithm. Maximum depth of recursion. Default to 1. */
int g_minimaxDepth = 1;

/* Is the user the black color (true) or white color (false). Default to White. */
bool g_isUserBlack = false;

/* A buffer allocated for reading user input. The user's command will not exceed 50 characters. */
char g_inputLine[LINE_LENGTH] = { 0 };

/* True if there was an allocation error somewhere in the program (would cause an exit). Else false. */
bool g_memError = false;

/* -- Functions -- */

/* A "toString()" function for Move structs */
void printMove(Move* move)
{
	printf("<%c,%d> to ", 'a' + move->initPos.x, move->initPos.y + 1);

	Node* currPosNode = move->nextPoses->head;

	while (NULL != currPosNode)
	{
		Position* currPos = (Position*)currPosNode->data;
		printf("<%c,%d>", 'a' + currPos->x, currPos->y + 1);

		currPosNode = currPosNode->next;
	}

	printf("\n");
}

/* Returns if the 2 positions are equal in their content. */
bool isEqualPositions(Position* a, Position* b)
{
	return (a->x == b->x) && (a->y == b->y);
}

/* Returns if the 2 moves are equal in their content. */
bool isEqualMoves(Move* a, Move* b)
{
	// Check startPos
	if (!isEqualPositions(&a->initPos, &b->initPos))
		return false;

	Node* currANode = a->nextPoses->head;
	Node* currBNode = b->nextPoses->head;

	// Check all destination positions
	while ((currANode != NULL) && (currBNode != NULL))
	{
		Position* currAPos = (Position*)currANode->data;
		Position* currBPos = (Position*)currBNode->data;

		if (!isEqualPositions(currAPos, currBPos))
			return false;

		currANode = currANode->next;
		currBNode = currBNode->next;
	}

	// Both moves should contain the same number of destinations
	return ((NULL == currANode) && (NULL == currBNode));
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
			return 0;
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
	int totalWhite = whiteArmy.pawns + whiteArmy.kings;
	Army blackArmy = getArmy(board, true);
	int totalBlack = blackArmy.pawns + blackArmy.kings;

	if (((whiteArmy.pawns == 0) && (whiteArmy.kings == 0)) 	// The board is empty
		|| ((blackArmy.pawns == 0) && (blackArmy.kings == 0))	// or there are discs of only one color
		|| (totalWhite > MAX_SOLDIERS) || (totalBlack > MAX_SOLDIERS)	// There are more than 20 discs of the same color
		|| (!validEdges(board)))	// There is a man in the opponent edge
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
		if (0 == strcmp(DIFFICULTY_DEPTH_COMMAND, args[0]))
		{	// Minimax depth
			int depth = atoi(args[1]);
			if ((depth >= 1) && (depth <= MAX_DEPTH))
			{
				g_minimaxDepth = depth;
			}
			else
			{	// Illegal depth
				printf(WRONG_MINIMAX_DEPTH);
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(USER_COLOR_COMMAND, args[0]))
		{	// User color
			if (0 == strcmp("white", args[1]))
			{
				g_isUserBlack = false;
			}
			else if (0 == strcmp("black", args[1]))
			{
				g_isUserBlack = true;
			}

			commandResult = RETRY;
		}
		else if (0 == strcmp(CLEAR_COMMAND, args[0]))
		{	// Clear
			clearBoard(board);
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
					if (WHITE_P == *args[3])
					{	// Man
						board[pos.x][pos.y] = WHITE_P;
					}
					else if (WHITE_K == *args[3])
					{	// King
						board[pos.x][pos.y] = WHITE_K;
					}
				}
				else if (0 == strcmp("black", args[2]))
				{	// Set black
					if (WHITE_P == *args[3])
					{	// Man
						board[pos.x][pos.y] = BLACK_P;
					}
					else if (WHITE_K == *args[3])
					{	// King
						board[pos.x][pos.y] = BLACK_K;
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
 * Performs validations on the parsed move: does the start position contain the player's piece and
 * is the given move legal (can the soldier move in this direction, is it the best move the player can execute, etc).
 */
bool validateMove(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack, Move* move)
{
	Position startPos = move->initPos;

	if (!isSquareOccupiedByCurrPlayer(board, isPlayerBlack, startPos.x, startPos.y))
	{ // Validation #2 - Piece does not belong to player
		printf(NO_PIECE);
		return false;
	}

	// Validation #3 - Is the move legal.
	// We compare the move against all legal moves (since player must always perform best eat, etc).
	LinkedList* possibleMoves = getMoves(board, isPlayerBlack);
	if (g_memError)
		return false;

	Node* currMoveNode = possibleMoves->head;
	bool isLegalMove = false;

	while ((!isLegalMove) && (NULL != currMoveNode))
	{
		Move* currMove = (Move*)currMoveNode->data;
		isLegalMove = isEqualMoves(move, currMove);
		currMoveNode = currMoveNode->next;
	}

	deleteList(possibleMoves); // Free resources

	if (!isLegalMove)
	{ // Validation #3 failed - This is not a legal move
		printf(ILLEGAL_MOVE);
		return false;
	}

	return true;
}

/* 
 * Parses and builds "Move" struct out of the arguments.
 * The Move will also be validated. If it is illegal, NULL is returned.
 * If the function succeeds and this is a valid move, the move is returned.
 */
Move* parseAndBuildMove(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack, char* args[MAX_ARGS])
{
	char* startPosArg = args[1];
	char* destPosArg = args[3];

	// First, we build the Move struct from the arguments
	Position startPos = argToPosition(startPosArg);

	if (!isSquareOnBoard(startPos.x, startPos.y))
	{ // Validation #1 - Invalid position
		printf(WRONG_POSITION);
		return NULL;
	}

	Move* move = createMove(startPos);
	if (g_memError)
		return NULL;

	// Fetch all destination positions in the chain.
	// We know we reached the end when instead of '<' we parse an EOF char.
	while (destPosArg[0] != '\0')
	{
		Position destPos = argToPosition(destPosArg);

		if (!isSquareOnBoard(destPos.x, destPos.y))
		{ // Validation #1 - Invalid position
			printf(WRONG_POSITION);
			deleteMove((void*)move);
			return NULL;
		}

		Position* newPos = createPosition(destPos.x, destPos.y);
		insertLast(move->nextPoses, newPos);
		if (g_memError)
		{
			deleteMove((void*)move);
			if (NULL != newPos)
				free(newPos);

			return NULL;
		}

		destPosArg += 5; // Advance to next destination in chain (if there's any)

		if (destPos.y == 9) // A string position of <x,10> is 1 character longer
			destPosArg++;
	}

	// 2 more validations remain: is the start position legal and is the move legal.
	// If an error occurs it is printed by the validation function.
	if (!validateMove(board, isPlayerBlack, move))
	{
		deleteMove((void*)move);
		return NULL;
	}

	if (g_memError)
	{
		deleteMove((void*)move);
		return NULL;
	}

	return move;
}

/* 
 * Executes a move command done by a player.
 * This function accepts a move and a board and updates the board.
 * In the end of this function the move is deleted.
 */
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

/* 
 * Executes a move command done by the user.
 * This function parses, builds, validates and executes the move.
 * Returns True if the move is legal and executed successfully.
 * If this move is impossible, False is returned.
 */
bool executeMoveCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack, char* args[MAX_ARGS])
{
	Move* move = parseAndBuildMove(board, isPlayerBlack, args);

	if (NULL == move) // Illegal move
		return false;

	// Move is valid and now contains all information we need to execute the next step
	executeMove(board, move);

	return true;
}

/* Prints a list of all possible moves for the player. */
void executeGetMovesCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack)
{
	LinkedList* possibleMoves = getMoves(board, isPlayerBlack);
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

/* Parse next user command during chess game and execute it. Return true if the game should end, false if not. */
COMMAND_RESULT parseUserCommand(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack)
{
	if (NULL == g_inputLine) // Avoid illegal NULL input
	{
		printf(ILLEGAL_COMMAND);
		return RETRY;
	}

	COMMAND_RESULT commandResult = RETRY;
	char* args[MAX_ARGS];
	int argc = breakInputToArgs(args);
	
	if (argc > 0)
	{
		if (0 == strcmp(QUIT_COMMAND, args[0]))
		{ // Quit
			commandResult = QUIT;
		}
		else if (0 == strcmp(GETMOVES_COMMAND, args[0]))
		{ // Get Moves
			executeGetMovesCommand(board, isPlayerBlack);
			commandResult = RETRY;
		}
		else if (0 == strcmp(MOVE_COMMAND, args[0]))
		{ // Move
			bool isMoveExecuted = executeMoveCommand(board, isPlayerBlack, args);

			if (!isMoveExecuted)
				commandResult = RETRY;
			else
				commandResult = SUCCESS;
		}
		else
		{ // Illegal command
			printf(ILLEGAL_COMMAND);
			commandResult = RETRY;
		}
	}
	else
	{ // Illegal command
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
bool executeUserTurn(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack)
{
	COMMAND_RESULT command = RETRY;

	while (RETRY == command)
	{
		printf(ENTER_YOUR_MOVE);
		
		getUserInput();

		command = parseUserCommand(board, isPlayerBlack);
	}

	return (QUIT == command);
}

/* Executes the next turn done by the computer. */
void executeComputerTurn(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack)
{
	// Compute next move by computer using the minimax algorithm
	bool isComputerBlack = !isPlayerBlack;
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
 * Executes the game loop.
 * The user and the computer each take turns until one of them wins or the user quits.
 * This function runs an entire single Chess game.
 * Input:
 *		board ~ The initial game board.
 *		isUserBlack ~ Whether the user is black (True) or white (False).
 */
void executeGameLoop(char board[BOARD_SIZE][BOARD_SIZE], bool isUserBlack)
{
	bool isUserTurn = !isUserBlack;
	bool isBlackTurn = false;
	bool isQuit = false;

	// Treat the edge case of a game board where one player immediately loses due to a non-fair game setting
	if (isPlayerVictor(board, true))
	{ // Black wins
		printf(WIN_MSG, BLACK_STR);
		return;
	}
	if (isPlayerVictor(board, false))
	{ // White wins
		printf(WIN_MSG, WHITE_STR);
		return;
	}

	// Game loops while the user doesn't quit and there's no victor
	while (!isQuit)
	{
		// Game continues, execute next turn
		if (isUserTurn)
		{
			isQuit = executeUserTurn(board, isUserBlack);
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

			// Check for victory - to see if game ends
			if (isPlayerVictor(board, isBlackTurn))
			{
				printf(WIN_MSG, isBlackTurn ? BLACK_STR : WHITE_STR);
				isQuit = true;
			}
			else
			{
				// Change turns
				isBlackTurn = !isBlackTurn;
				isUserTurn = !isUserTurn;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	print_board(board);

	printf(WELCOME_TO_CHESS);

	// Start game settings mode.
	// If the user doesn't quit, we start the game (determineGameSettings returns true).
	if (determineGameSettings(board))
	{
		executeGameLoop(board, g_isUserBlack);
	}

	return 0;
}