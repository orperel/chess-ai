#ifndef TYPES_
#define TYPES_

#include "LinkedList.h"

/* Define bool for c language. */
typedef enum { false, true } bool;

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

#endif