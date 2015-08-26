#include <stdlib.h>
#include "BoardManager.h"
#include "GameLogic.h"

/* -- Forward Declarations -- */

void queryAdditionalEats(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
						 bool isMovesForBlackPlayer, Move* currMove, int* minNumOfDisksRemoved, bool isSingleEatMode);

/* -- Functions -- */

/*
* Checks if a man can move to the current square.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves. This list will always contain the best moves so far.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the soldier is currently located.
*		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
*							   This helps us determine we return only the best moves possible for the player.
*		diagX, diagY ~ The target square the man wants to move to (x,y).
*/
void queryManMoveSquare(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
	bool isMovesForBlackPlayer, Position startPos, int* minNumOfDisksRemoved,
	int diagX, int diagY)
{
	// Man move counts only if:
	// 1) The next square is vacant.
	// 2) There are no eat moves available.
	if ((isSquareVacant(board, diagX, diagY)) && (*minNumOfDisksRemoved == 0))
	{
		Move* newMove = createMove(startPos);
		if (g_memError)
			return;

		Position* targetPos = createPosition(diagX, diagY);
		if (g_memError)
		{
			deleteMove((void*)newMove);
			return;
		}

		insertLast(newMove->nextPoses, targetPos);
		if (g_memError)
		{
			free(targetPos);
			deleteMove((void*)newMove);
			return;
		}

		insertLast(possibleMoves, newMove);
		if (g_memError)
		{
			deleteMove((void*)newMove);
			return;
		}
	}
}

/*  
 * Checks if the next square yields an eat move.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		currMove ~ Contains the last move done by the current soldier (in case of a chain eat),
 *				   or simply where it is located (first eat).
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *		enemyDiag, nextDiag ~ The square on which the next enemy *might* reside, and the square following it.
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void queryEatNextSquare(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
						 bool isMovesForBlackPlayer, Move* currMove, int* minNumOfDisksRemoved,
						 Position enemyDiag, Position nextDiag, bool isSingleEatMode)
{
	int enemyDiagX = enemyDiag.x;
	int enemyDiagY = enemyDiag.y;
	int nextDiagX = nextDiag.x;
	int nextDiagY = nextDiag.y;

	// Treat the edge case of cyclic move: we return to the starting point
	bool isCyclicMove = (nextDiagX == currMove->initPos.x) && (nextDiagY == currMove->initPos.y);

	// Can we eat a nearby enemy
	if (isSquareOccupiedByEnemy(board, isMovesForBlackPlayer, enemyDiagX, enemyDiagY) &&
		(isSquareVacant(board, nextDiagX, nextDiagY) || isCyclicMove))
	{
		Move* newMove = cloneMove(currMove);
		if (g_memError)
			return;

		Position* targetPos = createPosition(nextDiagX, nextDiagY);
		if (g_memError)
		{
			deleteMove((void*)newMove);
			return;
		}

		insertLast(newMove->nextPoses, targetPos);
		if (g_memError)
		{
			deleteMove((void*)newMove);
			free(targetPos);
			return;
		}

		Move* newMoveForList = NULL; // Will contain the clone we add to possibleMoves, in case we add the new move

		// Move has same eat count as new move we counted
		if (*minNumOfDisksRemoved == newMove->nextPoses->length)
		{
			// Clone and save to list of possible moves.
			// We clone here to avoid the case where the list gets deleted, but we still want to maintain
			// the pointer to newMove somewhere deeper in the recursion tree.
			newMoveForList = cloneMove(newMove);
			if (g_memError)
			{
				deleteMove((void*)newMove);
				return;
			}

			insertLast(possibleMoves, newMoveForList); // Concat move to moves list
		}
		else if (*minNumOfDisksRemoved < newMove->nextPoses->length)
		{ // New best move
			// This new move is better than all the old moves so it overrides them
			*minNumOfDisksRemoved = newMove->nextPoses->length;
			deleteAllNodes(possibleMoves);

			// Clone and save to list of possible moves.
			// We clone here to avoid the case where the list gets deleted, but we still want to maintain
			// the pointer to newMove somewhere deeper in the recursion tree.
			newMoveForList = cloneMove(newMove);
			if (g_memError)
			{
				deleteMove((void*)newMove);
				return;
			}

			insertLast(possibleMoves, newMoveForList); // Add move to moves list, this is the single current move
		}

		if (g_memError)
		{
			if (newMoveForList != NULL)
				deleteMove(newMoveForList);

			deleteMove((void*)newMove);
			return;
		}

		// We don't change the location of the original soldier during this calculation.
		// Only eaten soldiers.
		char soldier = board[newMove->initPos.x][newMove->initPos.y];

		// We don't recurse in single eat mode (where we want only the first eat).
		// Also - becoming a king stops any additional actions during this move.
		if ((!isSingleEatMode) && (!isBecomeKing(soldier, *targetPos)))
		{
			// Temporarily remove eaten soldier so we don't calculate it again for recursive moves
			char eatenEnemy = board[enemyDiagX][enemyDiagY];
			board[enemyDiagX][enemyDiagY] = EMPTY;

			// Checks is we can chain additional eats
			queryAdditionalEats(board, possibleMoves, isMovesForBlackPlayer, newMove,
								minNumOfDisksRemoved, isSingleEatMode);

			board[enemyDiagX][enemyDiagY] = eatenEnemy;
		}

		deleteMove((void*)newMove);
	}
}

/*  
 * Checks if we can chain additional eats from the current move.
 * Note: a current move may also be "an empty move", meaning, the soldier is just standing at the initial position.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		currMove ~ Contains the last move done by the current soldier (in case of a chain eat),
 *				   or simply where it is located (first eat).
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void queryAdditionalEats(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves, 
						 bool isMovesForBlackPlayer, Move* currMove, int* minNumOfDisksRemoved, bool isSingleEatMode)
{
	// The last square the soldier is on
	Position currSquare;
	
	if (currMove->nextPoses->length > 0)
	{
		Position* lastPos = (Position*)currMove->nextPoses->tail->data;
		currSquare.x = lastPos->x;
		currSquare.y = lastPos->y;
	}
	else
	{
		currSquare.x = currMove->initPos.x;
		currSquare.y = currMove->initPos.y;
	}

	// Check the 4 possible directions
	Position enemyDiag;
	Position nextDiag;
	enemyDiag.x = currSquare.x - 1;
	enemyDiag.y = currSquare.y + 1;
	nextDiag.x = currSquare.x - 2;
	nextDiag.y = currSquare.y + 2;
	queryEatNextSquare(board, possibleMoves, isMovesForBlackPlayer, currMove,
					   minNumOfDisksRemoved, enemyDiag, nextDiag, isSingleEatMode);
	if (g_memError)
		return;

	enemyDiag.x = currSquare.x + 1;
	enemyDiag.y = currSquare.y + 1;
	nextDiag.x = currSquare.x + 2;
	nextDiag.y = currSquare.y + 2;
	queryEatNextSquare(board, possibleMoves, isMovesForBlackPlayer, currMove,
					   minNumOfDisksRemoved, enemyDiag, nextDiag, isSingleEatMode);
	if (g_memError)
		return;

	enemyDiag.x = currSquare.x - 1;
	enemyDiag.y = currSquare.y - 1;
	nextDiag.x = currSquare.x - 2;
	nextDiag.y = currSquare.y - 2;
	queryEatNextSquare(board, possibleMoves, isMovesForBlackPlayer, currMove,
					   minNumOfDisksRemoved, enemyDiag, nextDiag, isSingleEatMode);
	if (g_memError)
		return;

	enemyDiag.x = currSquare.x + 1;
	enemyDiag.y = currSquare.y - 1;
	nextDiag.x = currSquare.x + 2;
	nextDiag.y = currSquare.y - 2;
	queryEatNextSquare(board, possibleMoves, isMovesForBlackPlayer, currMove,
					   minNumOfDisksRemoved, enemyDiag, nextDiag, isSingleEatMode);
}

/*
 * Get all possible position / eat moves of a man soldier.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the soldier is currently located.
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void getLegalManMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					 bool isMovesForBlackPlayer, Position startPos, int* minNumOfDisksRemoved, bool isSingleEatMode)
{
	// Check 2 forward move directions
	int diagX;
	int diagY;

	if (isMovesForBlackPlayer)
		diagY = startPos.y - 1; // Black player advances downwards
	else
		diagY = startPos.y + 1; // White player advances upwards

	diagX = startPos.x - 1;

	queryManMoveSquare(board, possibleMoves, isMovesForBlackPlayer,
					   startPos, minNumOfDisksRemoved, diagX, diagY);
	if (g_memError)
		return;

	diagX = startPos.x + 1;

	queryManMoveSquare(board, possibleMoves, isMovesForBlackPlayer,
						 startPos, minNumOfDisksRemoved, diagX, diagY);
	if (g_memError)
		return;

	// Check possible eats as well
	Move* baseMove = createMove(startPos);
	if (g_memError)
		return;

	queryAdditionalEats(board, possibleMoves, isMovesForBlackPlayer, baseMove, minNumOfDisksRemoved, isSingleEatMode);
	deleteMove((void*)baseMove);
}

/* 
 * Checks if a king can move / eat in the given direction.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the soldier is currently located.
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *		deltaX, deltaY ~ The direction the king advances in (|deltaX|=|deltaY|=1, sign determines direction).
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void queryKingDirection(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
						 bool isMovesForBlackPlayer, Position startPos, int* minNumOfDisksRemoved,
						 int deltaX, int deltaY, bool isSingleEatMode)
{
	Position currentSquare = startPos;
	currentSquare.x += deltaX;
	currentSquare.y += deltaY;

	// We advance along the diagonal, advancing by deltaX, Y each iteration.
	// We will stop once we no longer hit an empty square.
	while (isSquareVacant(board, currentSquare.x, currentSquare.y))
	{
		// If we can get eat moves for the current turn, we don't bother with position moves
		if ((*minNumOfDisksRemoved == 0))
		{
			Move* newMove = createMove(startPos);
			if (g_memError)
				return;

			Position* targetPos = createPosition(currentSquare.x, currentSquare.y);
			if (g_memError)
			{
				deleteMove((void*)newMove);
				return;
			}

			insertLast(newMove->nextPoses, targetPos);
			if (g_memError)
			{
				free(targetPos);
				deleteMove((void*)newMove);
				return;
			}

			insertLast(possibleMoves, newMove);
			if (g_memError)
			{
				deleteMove((void*)newMove);
				return;
			}
		}

		currentSquare.x += deltaX;
		currentSquare.y += deltaY;
	}

	// If the reason we stopped iterating was we encountered an enemy soldier, and the next square after it is vacant,
	// this is a legal eat. We start calculating eats here.
	Position enemyDiag;
	Position nextDiag;
	enemyDiag.x = currentSquare.x;
	enemyDiag.y = currentSquare.y;
	nextDiag.x = currentSquare.x + deltaX;
	nextDiag.y = currentSquare.y + deltaY;
	Move* baseMove = createMove(startPos);
	if (g_memError)
		return;

	// After the first eat, the king behaves like a man soldier
	queryEatNextSquare(board, possibleMoves, isMovesForBlackPlayer, baseMove,
					   minNumOfDisksRemoved, enemyDiag, nextDiag, isSingleEatMode);
	deleteMove((void*)baseMove);
}

/*
 * Get all possible position / eat moves of a king soldier.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the soldier is currently located.
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void getLegalKingMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					  bool isMovesForBlackPlayer, Position startPos, int* minNumOfDisksRemoved, bool isSingleEatMode)
{
	// Check move / eat in 4 directions:
	int diagX = -1;
	int diagY = 1;
	queryKingDirection(board, possibleMoves, isMovesForBlackPlayer,
						startPos, minNumOfDisksRemoved, diagX, diagY, isSingleEatMode);
	if (g_memError)
		return;

	if (isSingleEatMode && (possibleMoves->length > 0))
		return; // Optimization - if we've found possible moves, no need to keep checking

	diagX = 1;
	diagY = 1;
	queryKingDirection(board, possibleMoves, isMovesForBlackPlayer,
						startPos, minNumOfDisksRemoved, diagX, diagY, isSingleEatMode);
	if (g_memError)
		return;

	if (isSingleEatMode && (possibleMoves->length > 0))
		return; // Optimization - if we've found possible moves, no need to keep checking

	diagX = 1;
	diagY = -1;
	queryKingDirection(board, possibleMoves, isMovesForBlackPlayer,
						startPos, minNumOfDisksRemoved, diagX, diagY, isSingleEatMode);
	if (g_memError)
		return;

	if (isSingleEatMode && (possibleMoves->length > 0))
		return; // Optimization - if we've found possible moves, no need to keep checking

	diagX = -1;
	diagY = -1;
	queryKingDirection(board, possibleMoves, isMovesForBlackPlayer,
						startPos, minNumOfDisksRemoved, diagX, diagY, isSingleEatMode);
}

/*
 * Get all possible position / eat moves of any soldier.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves. This list will always contain the best moves so far.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the soldier is currently located.
 *		minNumOfDisksRemoved ~ The minimum number of disks removed by the current eat chain for this move to count.
 *							   This helps us determine we return only the best moves possible for the player.
 *      isSingleEatMode ~ Determines if we should recurse and keep looking for chained eats.
 *						  This parameter can be true and then the function doesn't recurse (and returns only the first eat).
 */
void getLegalPieceMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					   bool isMovesForBlackPlayer, Position startPos, int* minNumOfDisksRemoved, bool isSingleEatMode)
{
	if (isSquareOccupiedByCurrPlayer(board, isMovesForBlackPlayer, startPos.x, startPos.y))
	{
		if (isSquareOccupiedByKing(board, startPos.x, startPos.y))
		{
			getLegalKingMove(board, possibleMoves, isMovesForBlackPlayer,
							 startPos, minNumOfDisksRemoved, isSingleEatMode);
		}
		else
		{
			getLegalManMove(board, possibleMoves, isMovesForBlackPlayer,
						    startPos, minNumOfDisksRemoved, isSingleEatMode);
		}
	}
}

/* 
 * Iterates the board and returns a list of moves the player can make with each soldier
 * Input:
 *		board ~ The game board.
 *		isMovesForBlackPlayer ~ True if the function returns moves for the black player.
 *							    False if the function returns moves for the white player.
 */
LinkedList* getMoves(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer)
{
	// Algorithm:
	// 0) Create an empty list of possible moves. Lets call it possibleMoves.
	//    Set MinNumOfDisksRemoved = 0.
	// 1) Iterate the board - only black squares can have soldiers so we can jump every 2 squares
	//    (beware of the board end).
	// 2) If the current soldier belongs to the player (isBlackPlayer and current player is black, etc).
	//		2.1) Call getLegalManMove / getLegalKingMove with the current position.
	//			 Add the results to possibleMoves (depending on the soldier type).
	// 3) Return possibleMoves.

	LinkedList* possibleMoves = createList(*deleteMove); // <-- This list contains the results of best moves available.
													     // Note we change this list in the following service functions.
	if (g_memError)
		return NULL;

	int minNumOfDisksRemoved = 0;
	int i, j; // i = column, j = row

	for (j = BOARD_SIZE - 1; j >= 0; j--)
	{
		bool isEvenRow = (j % 2 == 0);
		for (i = (isEvenRow) ? 0 : 1; i < BOARD_SIZE; i += 2)
		{
			Position startPos;
			startPos.x = i;
			startPos.y = j;
			getLegalPieceMove(board, possibleMoves, isMovesForBlackPlayer, startPos, &minNumOfDisksRemoved, false);

			if (g_memError)
			{
				deleteList(possibleMoves);
				return NULL;
			}
		}
	}

	return possibleMoves;
}

/* 
 * Iterates the board and returns whether the player can make any more move (false) or he is stuck (true).
 * This function is more optimized than getMoves() - it will stop as soon as possible moves are found.
 * Also - chained eats won't be checked.
 * Input:
 *		board ~ The game board.
 *		isBlackPlayer ~ True if the function checks moves for the black player.
 *					    False if the function checks moves for the white player.
 */
bool isPlayerStuck(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackPlayer)
{
	LinkedList* possibleMoves = createList(*deleteMove); // <-- This list contains the results of best moves available.
														 // Note we change this list in the following service functions.
	if (g_memError)
		return false;

	int minNumOfDisksRemoved = 0; // We don't really need to calculate the best eats available,
								  // but the service functions we use require this parameter so we supply it

	int i, j; // i = column, j = row
	bool isMovesFound = false;

	// We iterate the board as long as we don't find any possible moves
	for (j = BOARD_SIZE - 1; !isMovesFound && (j >= 0); j--)
	{
		bool isEvenRow = (j % 2 == 0);
		for (i = (isEvenRow) ? 0 : 1; !isMovesFound && (i < BOARD_SIZE); i += 2)
		{
			Position startPos;
			startPos.x = j;
			startPos.y = i;
			getLegalPieceMove(board, possibleMoves, isBlackPlayer, startPos, &minNumOfDisksRemoved, true);

			isMovesFound = (possibleMoves->length > 0);

			if (g_memError)
			{
				deleteList(possibleMoves);
				return false;
			}
		}
	}

	deleteList(possibleMoves);

	return !isMovesFound;
}

/* 
 * Checks the board to see if the black / white player has won.
 * Input:
 *		board ~ The game board.
 *		isPlayerBlack ~ Check if the black player has won (true) or if the white player has won (false).
 */
bool isPlayerVictor(char board[BOARD_SIZE][BOARD_SIZE], bool isPlayerBlack)
{
	// Check if the other player's army is gone
	Army enemyArmy = isPlayerBlack ? getArmy(board, false) : getArmy(board, true);
	if ((enemyArmy.pawns == 0) && (enemyArmy.kings == 0))
		return true;

	// Check if the enemy can make any more moves
	if (isPlayerStuck(board, !isPlayerBlack))
		return true;

	return false;
}