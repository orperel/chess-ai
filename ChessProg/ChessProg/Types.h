#ifndef TYPES_
#define TYPES_

/* Define bool for c language. */
typedef enum { false, true } bool;

/** Error code returned when the framework terminates successfully */
#define OK_EXIT_CODE 0

/** Error code returned when SDL fails to initialize */
#define SDL_FAILURE_EXIT_CODE 1

/** Error code returned when a gui error occures */
#define GUI_ERROR_EXIT_CODE 2

/** Error code returned when a gui error occures */
#define MEMORY_ERROR_EXIT_CODE 3

/** Definitions for chess game board representations */
#define WHITE_P 'm'
#define WHITE_B 'b'
#define WHITE_N 'n'
#define WHITE_R 'r'
#define WHITE_Q 'q'
#define WHITE_K 'k'
#define BLACK_P 'M'
#define BLACK_B 'B'
#define BLACK_N 'N'
#define BLACK_R 'R'
#define BLACK_Q 'Q'
#define BLACK_K 'K'
#define EMPTY ' '
#define BOARD_SIZE 8
#define INVALID_POSITION_INDEX -1
#define MAX_ARGS 7			// Max number of args supported by shell
#define LINE_LENGTH 50		// Shell "buffer" size used to read user input
#define MAX_DEPTH 4			// Maximum depth miniMax algorithm depth can be
#define MAX_SOLDIERS 16		// Maximum number of soldiers per player
#define GAME_MODE_2_PLAYERS 1
#define GAME_MODE_PLAYER_VS_AI 2
#define WRONG_MINIMAX_DEPTH "Wrong value for minimax depth. The value should be between 1 to 4\n"
#define WRONG_FILE_NAME "Wrong file name\n"
#define WRONG_POSITION "Invalid position on the board\n"
#define WRONG_SET "Setting this piece creates an invalid board\n"  
#define NO_PIECE "The specified position does not contain your piece\n"
#define BLACK_STR "Black"
#define WHITE_STR "White"
#define DIFFICULTY_BEST "best"

// Results of a user command
typedef enum { SUCCESS, RETRY, QUIT } COMMAND_RESULT;

/* A position on the game board. */
typedef struct
{
	int x, y;
} Position;

/* A single move made by a soldier on the game board (all routes are included in this move). */
typedef struct
{
	Position initPos;	   // Initial position in beginning of move
	Position nextPos;      // The next position the soldier moves to during the move

	char promotion;		   // For moves that transform pieces (e.g: pawn arrives edge of board),
						   // this field states which piece the current piece transforms into.
						   // EMPTY symbolizes no promotion.
} Move;

/* Capacity of soldiers of a player. */
typedef struct
{
	int pawns;
	int bishops;
	int rooks;
	int knights;
	int queens;
	int kings;
} Army;

/* 
 * Represents a single game step done by a player.
 * This struct contains the list of changes made to the game state in this step.
 */
typedef struct
{
	char currSoldier;				// 'm', 'M', 'b', 'B', 'r', 'R', 'n', 'N', 'q', 'Q', 'k' or 'K'.
	bool isStepByBlackPlayer;		// True if the step was done by the black player. False if done by white.
	Position startPos;				// Where the soldier was located at beginning of step.
	Position endPos;				// Where the soldier was located at the end of the step.
	char promotion;					// EMPTY if no promotion for a pawn occured in this step.
									// Otherwise contains the promotion: 'b', 'B', 'r', 'R', 'n', 'N', 'q', 'Q'.
	bool isEnemyRemovedInStep;		// True if an enemy piece was eaten on this step. False if not.
									// The position of the enemy eaten is the same as endPos.
	char removedType;				// Types of enemy soldier removed. Relevant only if isEnemyRemovedInStep==true.
} GameStep;


/* -- Globals Declaration -- */

/* The game mode. There are two possible values: 1 - two players mode, and 2 - player vs. AI mode. Default to 1. */
extern int g_gameMode;

/* Configuration for mini-max algorithm. Maximum depth of recursion. Default to 1. */
extern int g_minimaxDepth;

/* Is the difficulty best - i.e. the depth of recursion alternates. Default to false. */
extern bool g_isDifficultyBest;

/* Is the user the black color (true) or white color (false). Default to white. */
extern bool g_isUserBlack;

/* Is the next player the black color (true) or white color (false). Default to white. */
extern bool g_isNextPlayerBlack;

/* A buffer allocated for reading user input. The user's command will not exceed 50 characters. */
extern char g_inputLine[LINE_LENGTH];

/* True if there was an allocation error somewhere in the program (would cause an exit). Else false. */
extern bool g_memError;

/* The boards counter for the Minimax algorithm with difficulty best. */
extern int g_boardsCounter;

/* -- General functions -- */

/** Set all the global variables to their default. */
void initGlobals();

/** A general max function for integers (that doesn't use macros) */
int maxi(int a, int b);

/** A general min function for integers (that doesn't use macros) */
int mini(int a, int b);

#endif