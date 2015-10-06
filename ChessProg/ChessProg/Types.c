#include "Types.h"

/* -- Globals Definition -- */

/* The game mode. There are two possible values: 1 - two players mode, and 2 - player vs. AI mode. Default to 1. */
int g_gameMode = 1;

/* Configuration for mini-max algorithm. Maximum depth of recursion. Default to 1. */
int g_minimaxDepth = 1;

/* Is the difficulty best - i.e. the depth of recursion alternates. Default to false. */
bool g_isDifficultyBest = false;

/* Is the user the black color (true) or white color (false). Default to white. */
bool g_isUserBlack = false;

/* Is the next player the black color (true) or white color (false). Default to white. */
bool g_isNextPlayerBlack = false;

/* A buffer allocated for reading user input. The user's command will not exceed 50 characters. */
char g_inputLine[LINE_LENGTH] = { 0 };

/* True if there was an allocation error somewhere in the program (would cause an exit). Else false. */
bool g_memError = false;

/* The boards counter for the Minimax algorithm with difficulty best. */
int g_boardsCounter = 0;


/* -- General functions -- */

/** A general max function for integers (that doesn't use macros) */
int maxi(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

/** A general min function for integers (that doesn't use macros) */
int mini(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}