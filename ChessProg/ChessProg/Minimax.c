#include <limits.h>
#include "Chess.h"
#include "BoardManager.h"
#include "GameLogic.h"
#include "Minimax.h"

/* Compute the total score of the given board and player. */
int getScore(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack)
{
	Army whiteArmy = getArmy(board, false);
	int whiteScore = (whiteArmy.pawns * PAWN_SCORE) + (whiteArmy.bishops * BISHOP_SCORE) + (whiteArmy.rooks * ROOK_SCORE)
					 + (whiteArmy.knights * KNIGHT_SCORE) + (whiteArmy.queens * QUEEN_SCORE) + (whiteArmy.kings * KING_SCORE);
	Army blackArmy = getArmy(board, true);
	int blackScore = (blackArmy.pawns * PAWN_SCORE) + (blackArmy.bishops * BISHOP_SCORE) + (blackArmy.rooks * ROOK_SCORE)
					 + (blackArmy.knights * KNIGHT_SCORE) + (blackArmy.queens * QUEEN_SCORE) + (blackArmy.kings * KING_SCORE);
	
	if (!isABlack)
	{	// White turn
		return (whiteScore - blackScore);
	}
	else
	{	// Black turn
		return (blackScore - whiteScore);
	}
}

/* 
 * Implement the Alphabeta pruning algorithm to decrease the number of nodes that are evaluated by the Minimax.
 * If there was an error set g_memError to true and return INT_MIN. 
 */
int alphabeta(char board[BOARD_SIZE][BOARD_SIZE], int level, int alpha, int beta, bool isABlack)
{
	// Reach Minimax depth (leaf)
	if (level == g_minimaxDepth)
	{
		return getScore(board, !isABlack); // Return score as for the parent level in the tree
	}

	LinkedList* moves = getMoves(board, isABlack);
	if (g_memError)
	{	// Error
		return INT_MIN;
	}

	// Tie move (leaf). Return worst score, except for loosing, for the parent.
	if (moves->length == 0)
	{
		deleteList(moves);
		
		if ((level % 2) == 0)
		{	// Max turn
			return TIE_SCORE_ABS;
		}
		else
		{	// Min turn
			return -TIE_SCORE_ABS;
		}
	}

	int value;
	bool stuckResult;
	if ((level % 2) == 0)
	{	// Max turn
		value = INT_MIN;
		
		Node* currMove = moves->head;
		while ((currMove != NULL) && (beta > alpha))
		{
			Move* currMoveData = (Move*)(currMove->data);
			GameStep* currGameStep = createGameStep(board, currMoveData);	// Convert Move to gameStep
			if (g_memError)
			{	// Error
				deleteList(moves);
				return INT_MIN;
			}
			doStep(board, currGameStep);

			// Check if the opponent is stuck. If yes, stop recursion in this branch of the tree
			stuckResult = false;// TODO: isPlayerStuck(board, !isABlack);
			if (g_memError)
			{
				undoStep(board, currGameStep);
				deleteGameStep(currGameStep);
				deleteList(moves);
				return INT_MIN;
			}
			if (stuckResult)
			{
				undoStep(board, currGameStep);
				deleteGameStep(currGameStep);
				deleteList(moves);
				return WINNING_SCORE;
			}

			// Call alphabeta algorithm on the current move (child)
			value = alphabeta(board, level + 1, alpha, beta, !isABlack); 

			// Max between alpha and value 
			if (alpha < value)
			{
				alpha = value;
			}

			undoStep(board, currGameStep);
			deleteGameStep(currGameStep);
			currMove = currMove->next;
		}
	}
	else
	{	// Min turn
		value = INT_MAX;

		Node* currMove = moves->head;
		while ((currMove != NULL) && (beta > alpha))
		{
			Move* currMoveData = (Move*)(currMove->data);
			GameStep* currGameStep = createGameStep(board, currMoveData);	// Convert Move to gameStep
			if (g_memError)
			{	// Error
				deleteList(moves);
				return INT_MIN;
			}
			doStep(board, currGameStep);

			// Check if the opponent is stuck. If yes, stop recursion in this branch of the tree
			stuckResult = false;// TODO: isPlayerStuck(board, !isABlack);
			if (g_memError)
			{
				undoStep(board, currGameStep);
				deleteGameStep(currGameStep);
				deleteList(moves);
				return INT_MIN;
			}
			if (stuckResult)
			{
				undoStep(board, currGameStep);
				deleteGameStep(currGameStep);
				deleteList(moves);
				return LOOSING_SCORE;
			}

			// Call alphabeta algorithm on the current move (child)
			value = alphabeta(board, level + 1, alpha, beta, !isABlack);

			// Min between beta and value
			if (beta > value)
			{
				beta = value;
			}

			undoStep(board, currGameStep);
			deleteGameStep(currGameStep);
			currMove = currMove->next;
		}
	}

	deleteList(moves);
	return value;
}

/* 
 * Implement the Minimax algorithm.
 * If there was an error set g_memError to true and return NULL. 
 */
Move* minimax(char board[BOARD_SIZE][BOARD_SIZE], bool isABlack)
{
	LinkedList* moves = getMoves(board, isABlack);	// Get moves for current state
	if (g_memError)
	{	// Error
		return NULL;
	}

	// Init alpha, beta and value. Look for the max value among the children of the root
	int alpha = INT_MIN;
	int beta = INT_MAX;
	int maxValue = alpha;
	int value;

	Node* currMove = moves->head;
	Move* winMove = NULL;
	bool stuckResult;
	while ((currMove != NULL) && (beta > alpha))
	{
		Move* currMoveData = (Move*)(currMove->data);
		GameStep* currGameStep = createGameStep(board, currMoveData);	// Convert Move to gameStep
		if (g_memError)
		{	// Error
			deleteList(moves);
			return NULL;
		}
		doStep(board, currGameStep);
		
		// Check if the opponent is stuck. If yes, stop recursion in this branch of the tree
		stuckResult = false;// TODO: isPlayerStuck(board, !isABlack);
		if (g_memError)
		{
			undoStep(board, currGameStep);
			deleteGameStep(currGameStep);
			deleteList(moves);
			return NULL;
		}
		if (stuckResult)
		{
			undoStep(board, currGameStep);
			deleteGameStep(currGameStep);
			Move* returnedWinMove = cloneMove(currMoveData);
			deleteList(moves);
			return returnedWinMove;	// Win move
		}
	
		// Call alphabeta algorithm on the current move (child)
		value = alphabeta(board, 1, alpha, beta, !isABlack);
		if (g_memError)
		{
			undoStep(board, currGameStep);
			deleteGameStep(currGameStep);
			deleteList(moves);
			return NULL;
		}

		// Check if the current value is greater than the previous ones (the root is max turn)
		if (value > maxValue)
		{
			maxValue = value;
			winMove = currMoveData;	// Save winning move
		}

		// Max between alpha and value 
		if (alpha < value)
		{
			alpha = value;
		}

		undoStep(board, currGameStep);
		deleteGameStep(currGameStep);
		currMove = currMove->next;
	}

	winMove = cloneMove(winMove);

	deleteList(moves);
	return winMove;
}