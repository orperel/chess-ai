#include <stdlib.h>
#include "BoardManager.h"
#include "GameLogic.h"

/* -- Functions -- */

/*
* Returns true / false if a pawn is threatening the given king
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isPawnThreatningKing(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	// Test for threatening pawns
	// Check forward move
	int threatX = kingPos->x;
	int threatY = kingPos->y;

	if (isTestForBlackPlayer)
		threatX -= 1; // Black king is threatened by pawns below
	else
		threatX += 1; // Black king is threatened by pawns above

	return isSquareOccupiedByPawn(board, !isTestForBlackPlayer, threatX, threatY + 1) ||
		   isSquareOccupiedByPawn(board, !isTestForBlackPlayer, threatX, threatY - 1);
}

/*
* Returns true / false if a queen, rook or bishop threatens the king from the given direction
* Input:
*		board ~ The game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*		deltaX, deltaY ~ The direction the threat advances in.
*/
bool isDirectionThreat(char board[BOARD_SIZE][BOARD_SIZE],
					   bool isTestForBlackPlayer, Position* kingPos, int deltaX, int deltaY)
{
	// First square on the diagonal / row / column
	Position currentSquare = { kingPos->x + deltaX, kingPos->y + deltaY };

	// We advance along the direction, advancing by deltaX, Y each iteration.
	// We will stop once we no longer hit an empty square.
	while (isSquareVacant(board, currentSquare.x, currentSquare.y))
	{
		currentSquare.x += deltaX;
		currentSquare.y += deltaY;
	}

	// If the reason we stopped iterating was we encountered an enemy piece,
	// we check if that enemy piece is a threat.
	// Note the queen threatens from all directions, bishop only from diagonal and rook from vertical & horizontal lines
	bool isBishopThreating = isSquareOccupiedByBishop(board, !isTestForBlackPlayer, currentSquare.x, currentSquare.y);
	bool isRookThreating = isSquareOccupiedByRook(board, !isTestForBlackPlayer, currentSquare.x, currentSquare.y);
	bool isQueenThreating = isSquareOccupiedByQueen(board, !isTestForBlackPlayer, currentSquare.x, currentSquare.y);
	bool isHorVertThreat = ((deltaX == 0) || (deltaY == 0));
	bool isDiagonalThreat = (abs(deltaX) == abs(deltaY));

	return ((isQueenThreating && (isHorVertThreat || isDiagonalThreat )) ||
		    (isBishopThreating && isDiagonalThreat) ||
			(isRookThreating && isHorVertThreat));
}

/*
* Returns true / false if a bishop or queen (diagonal) is threatening the given king
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isBishopQueenThreatningKing(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	return isDirectionThreat(board, isTestForBlackPlayer, kingPos, -1,  1)  ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos,  1, -1)  ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos,  1,  1)  ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos, -1, -1);
}

/*
* Returns true / false if a rook or queen (diagonal) is threatening the given king
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isRookQueenThreatningKing(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	return isDirectionThreat(board, isTestForBlackPlayer, kingPos,  0,  1) ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos,  1,  0) ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos,  0, -1) ||
		   isDirectionThreat(board, isTestForBlackPlayer, kingPos, -1,  0);
}

/*
* Returns true / false if a knight is threatening the given king
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isKnightThreatningKing(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	// The knight leaps forward in a "L shape" manner, therefore there are 8 possibilities
	return (isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x - 1, kingPos->y - 2) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x - 2, kingPos->y - 1) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x + 1, kingPos->y - 2) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x + 2, kingPos->y - 1) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x - 1, kingPos->y + 2) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x - 2, kingPos->y + 1) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x + 1, kingPos->y + 2) ||
			isSquareOccupiedByKnight(board, !isTestForBlackPlayer, kingPos->x + 2, kingPos->y + 1));
}

/*
* Returns true / false if a king is threatening the given king
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isKingThreatningKing(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	// The king can move one square up, down, left or right
	return (isSquareOccupiedByKing(board, !isTestForBlackPlayer, kingPos->x + 1, kingPos->y + 1) ||
			isSquareOccupiedByKing(board, !isTestForBlackPlayer, kingPos->x - 1, kingPos->y + 1) || 
			isSquareOccupiedByKing(board, !isTestForBlackPlayer, kingPos->x + 1, kingPos->y - 1) || 
			isSquareOccupiedByKing(board, !isTestForBlackPlayer, kingPos->x - 1, kingPos->y - 1));
}

/*
* Returns either whether the black player (isTestForBlackPlayer == true) is in check,
* or the white player (isTestForBlackPlayer == false) is in check.
* Input:
*		board ~ The chess game board.
*		isTestForBlackPlayer ~ True if check if black king is under check. False for white king.
*		kingPos ~ Where the king piece is currently located.
*/
bool isKingUnderCheck(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, Position* kingPos)
{
	return isPawnThreatningKing(board, isTestForBlackPlayer, kingPos) ||
		   isBishopQueenThreatningKing(board, isTestForBlackPlayer, kingPos) ||
		   isRookQueenThreatningKing(board, isTestForBlackPlayer, kingPos) ||
		   isKnightThreatningKing(board, isTestForBlackPlayer, kingPos) ||
		   isKingThreatningKing(board, isTestForBlackPlayer, kingPos);
}

/*
 * Returns if the move is valid (doesn't cause the current player a check).
 * Move is from startPos to target.
 * - Move is expected to already be valid in terms of piece type constraints
 *	(e.g: a peon can only move to 3 possible squares).
 * Input:
 *		board ~ The chess game board.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the piece is currently located.
 *		targetX, targetY ~ Coordinates of where the piece will move to.
 *		kingPos ~ Current position of the current player's king (following the execution of the move).
 */
bool isValidMove(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer,
				 Position* startPos, int targetX, int targetY, Position* kingPos)
{
	// First update the board as if the move is executed.
	// We don't bother taking peon promotion into consideration since this is irrelevant for the check validity test
	// (promoting a peon of the current player shouldn't cause the king of the current player be in check).
	char piece = board[startPos->x][startPos->y];
	char target = board[targetX][targetY];
	board[startPos->x][startPos->y] = EMPTY;
	board[targetX][targetY] = piece;

	bool isValid = !isKingUnderCheck(board, isMovesForBlackPlayer, kingPos);

	// Restore the board to its original state
	board[startPos->x][startPos->y] = piece;
	board[targetX][targetY] = target;

	return isValid;
}

/*
 * Add an available move for the player to the list of moves.
 * - Move is expected to be valid in terms of piece type constraints (e.g: a peon can only move to 3 possible squares).
 * - Additional validation will be done in this function (moves that result in a check status for the current player are
 *	 illegal).
 * --> If the move is legal, it is added to the list of possibleMoves. Otherwise nothing happens.
 * Input:
 *		board ~ The chess game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the piece is currently located.
 *		targetX, targetY ~ Coordinates of where the piece will move to.
 *		kingPos ~ Current position of the current player's king (following the execution of the move).
 */
bool addPossibleMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves, bool isMovesForBlackPlayer,
					 Position* startPos, int targetX, int targetY, Position* kingPos)
{
	// Check if the move doesn't cause the current player a check. If it does, we don't count it.
	if (!isValidMove(board, isMovesForBlackPlayer, startPos, targetX, targetY, kingPos))
		return false;

	Position targetPos = { targetX, targetY };

	Move* newMove = createMove(startPos, &targetPos);
	if (g_memError)
		return false;

	insertLast(possibleMoves, newMove);
	if (g_memError)
	{
		deleteMove((void*)newMove);
		return false;
	}

	return true;
}

/*
* Add an available move for the player to the list of moves, the move is specifically created for peons,
* as it may contain promotions.
* - Move is expected to be valid in terms of piece type constraints (e.g: a peon can only move to 3 possible squares).
* - Additional validation will be done in this function (moves that result in a check status for the current player are
*	 illegal).
* --> If the move is legal, it is added to the list of possibleMoves. Otherwise nothing happens.
* Input:
*		board ~ The chess game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		targetX, targetY ~ Coordinates of where the piece will move to.
*		kingPos ~ Current position of the current player's king (following the execution of the move).
*/
void addPeonMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves, bool isMovesForBlackPlayer,
					  Position* startPos, int targetX, int targetY, Position* kingPos)
{
	bool isMoveAdded = addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, targetX, targetY, kingPos);
	if (g_memError)
		return;
	if (!isMoveAdded)
		return;

	// If the pawn reaches the edge, the moves become promotion moves.
	if (isSquareOnOppositeEdge(isMovesForBlackPlayer, targetX))
	{
		char bishop = isMovesForBlackPlayer ? BLACK_B : WHITE_B;
		char rook = isMovesForBlackPlayer ? BLACK_R : BLACK_R;
		char knight = isMovesForBlackPlayer ? BLACK_N : WHITE_N;
		char queen = isMovesForBlackPlayer ? BLACK_Q : WHITE_Q;

		((Move*)(possibleMoves->tail->data))->promotion = queen; // Update the most recent move to a promotion move.

		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, targetX, targetY, kingPos);
		if (g_memError)
			return;
		((Move*)(possibleMoves->tail->data))->promotion = rook; // Update the most recent move to a promotion move.

		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, targetX, targetY, kingPos);
		if (g_memError)
			return;
		((Move*)(possibleMoves->tail->data))->promotion = bishop; // Update the most recent move to a promotion move.

		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, targetX, targetY, kingPos);
		if (g_memError)
			return;
		((Move*)(possibleMoves->tail->data))->promotion = knight; // Update the most recent move to a promotion move.
	}
}

/*
 * Get possible moves for current Pawn piece.
 * Input:
 *		board ~ The game board.
 *		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
 *						possible eat / position change moves.
 *		isMovesForBlackPlayer ~ True if current player is black. False if white.
 *		startPos ~ Where the piece is currently located.
 *		kingPos ~ Current position of the current player's king.
 */
void getPawnMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
				  bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// Check forward move
	int advanceX = startPos->x;
	int advanceY = startPos->y;

	if (isMovesForBlackPlayer)
		advanceX -= 1; // Black player advances downwards
	else
		advanceX += 1; // White player advances upwards

	// Check if the pawn can move forward to a vacant spot
	if (isSquareVacant(board, advanceX, advanceY))
	{
		addPeonMove(board, possibleMoves, isMovesForBlackPlayer, startPos, advanceX, advanceY, kingPos);
	}
	if (g_memError)
		return;

	// Check if the pawn can eat in 1st diagonal
	advanceY = startPos->y + 1;
	if (isSquareOccupiedByEnemy(board, isMovesForBlackPlayer, advanceX, advanceY))
		addPeonMove(board, possibleMoves, isMovesForBlackPlayer, startPos, advanceX, advanceY, kingPos);
	if (g_memError)
		return;

	// Check if the pawn can eat in 2nd diagonal
	advanceY = startPos->y - 1;
	if (isSquareOccupiedByEnemy(board, isMovesForBlackPlayer, advanceX, advanceY))
		addPeonMove(board, possibleMoves, isMovesForBlackPlayer, startPos, advanceX, advanceY, kingPos);
	if (g_memError)
		return;
}

/*
* Checks if a piece can move / eat in the given direction.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		deltaX, deltaY ~ The direction the piece advances in.
*		kingPos ~ Current position of the current player's king.
*/
void queryDirection(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					bool isMovesForBlackPlayer, Position* startPos, int deltaX, int deltaY, Position* kingPos)
{
	// First square on the diagonal / row / column
	Position currentSquare = { startPos->x + deltaX, startPos->y + deltaY };

	// We advance along the direction, advancing by deltaX, Y each iteration.
	// We will stop once we no longer hit an empty square.
	while (isSquareVacant(board, currentSquare.x, currentSquare.y))
	{
		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer,startPos, currentSquare.x, currentSquare.y, kingPos);
		if (g_memError)
			return;

		currentSquare.x += deltaX;
		currentSquare.y += deltaY;
	}

	// If the reason we stopped iterating was we encountered an enemy piece, we get an additional move: an eat move.
	if (isSquareOccupiedByEnemy(board, isMovesForBlackPlayer, currentSquare.x, currentSquare.y))
	{
		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, currentSquare.x, currentSquare.y, kingPos);
	}
}

/*
* Get possible moves for current Bishop piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		kingPos ~ Current position of the current player's king.
*/
void getBishopMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// Check move / eat in 4 diagonal directions:
	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, 1, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, 1, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, -1, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, -1, kingPos);
}

/*
* Get possible moves for current Rook piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		kingPos ~ Current position of the current player's king.
*/
void getRookMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
				  bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// Check move / eat in 4 cross directions:
	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, 0, 1, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, 0, -1, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, 0, kingPos);
	if (g_memError)
		return;

	queryDirection(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, 0, kingPos);
}

/*
* Add possible move for a single spot, if that spot is available for moving to or eating an enemy piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		deltaX, deltaY ~ How many squares away to move the piece to.
*		kingPos ~ Current position of the current player's king.
*/
void querySinglePos(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					bool isMovesForBlackPlayer, Position* startPos, int deltaX, int deltaY, Position* kingPos)
{
	Position nextSquare = { startPos->x + deltaX, startPos->y + deltaY  };

	if (isSquareVacant(board, nextSquare.x, nextSquare.y) ||
		isSquareOccupiedByEnemy(board, isMovesForBlackPlayer, nextSquare.x, nextSquare.y))
	{
		Position* kingPosArg;

		// When the king moves, we should update the kingPos parameter accordingly
		if ((startPos->x == kingPos->x) && (startPos->y == kingPos->y))
			kingPosArg = &nextSquare; // The king's updated position is where the king moves to
		else
			kingPosArg = kingPos; // The current move is not made by a king, the kingPos stays the same

		addPossibleMove(board, possibleMoves, isMovesForBlackPlayer, startPos, nextSquare.x, nextSquare.y, kingPosArg);
	}
}

/*
* Get possible moves for current Knight piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		kingPos ~ Current position of the current player's king.
*/
void getKnightMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
					bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// The knight leaps forward in a "L shape" manner, therefore there are 8 possibilities
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, 2, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 2, 1, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, 2, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -2, 1, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, -2, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 2, -1, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, -2, kingPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -2, -1, kingPos);
}

/*
* Get possible moves for current Queen piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located.
*		kingPos ~ Current position of the current player's king.
*/
void getQueenMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
				   bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// The queen combines the power of a bishop and a rook
	getBishopMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
	if (g_memError)
		return;
	getRookMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
}

/*
* Get possible moves for current King piece.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, we aggregate it as we check
*						possible eat / position change moves.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the piece is currently located. Also the current position of the current player's king.
*/
void getKingMoves(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
				  bool isMovesForBlackPlayer, Position* startPos)
{
	// The king can move one square in any direction.
	// Remember that for the king - startPos == kingPos..
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 0, 1, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 0, -1, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, 0, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, 0, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, 1, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, 1, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, 1, -1, startPos);
	if (g_memError)
		return;
	querySinglePos(board, possibleMoves, isMovesForBlackPlayer, startPos, -1, -1, startPos);
}

/*
* Get all possible position / eat moves of any soldier.
* Input:
*		board ~ The game board.
*		possibleMoves ~ A list of possible moves by the current player, the list will be filled with possible moves for
*					    the piece.
*		isMovesForBlackPlayer ~ True if current player is black. False if white.
*		startPos ~ Where the soldier is currently located.
*		kingPos ~ Current position of the current player's king.
*/
void getPieceMove(char board[BOARD_SIZE][BOARD_SIZE], LinkedList* possibleMoves,
				  bool isMovesForBlackPlayer, Position* startPos, Position* kingPos)
{
	// Search for moves only if the piece on the square belongs to the current player.
	if (isSquareOccupiedByCurrPlayer(board, isMovesForBlackPlayer, startPos->x, startPos->y))
	{
		switch (board[startPos->x][startPos->y]) // Get move by piece type
		{
			case (WHITE_P):
			case (BLACK_P):
			{
				getPawnMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
				break;
			}
			case (WHITE_B) :
			case (BLACK_B) :
			{
				getBishopMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
				break;
			}
			case (WHITE_R) :
			case (BLACK_R) :
			{
				getRookMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
				break;
			}
			case (WHITE_N) :
			case (BLACK_N) :
			{
				getKnightMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
				break;
			}
			case (WHITE_Q) :
			case (BLACK_Q) :
			{
				getQueenMoves(board, possibleMoves, isMovesForBlackPlayer, startPos, kingPos);
				break;
			}
			case (WHITE_K) :
			case (BLACK_K) :
			{
				getKingMoves(board, possibleMoves, isMovesForBlackPlayer, startPos);
				break;
			}
			default:
				break; // Illegal board piece
			}
	}
}

/* 
 * Iterates the board and returns a list of moves the player can make with each piece
 * Input:
 *		board ~ The game board.
 *		isMovesForBlackPlayer ~ True if the function returns moves for the black player.
 *							    False if the function returns moves for the white player.
 */
LinkedList* getMoves(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer)
{
	LinkedList* possibleMoves = createList(deleteMove);  // <-- This list contains the results of moves available.
													     // Note we change this list in the following service functions.
	if (g_memError)
		return NULL;

	Position kingPos = getKingPosition(board, isMovesForBlackPlayer); // Position of current player's king

	int i, j; // i = row, j = column

	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			Position startPos;
			startPos.x = i;
			startPos.y = j;
			getPieceMove(board, possibleMoves, isMovesForBlackPlayer, &startPos, &kingPos);

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
 * Returns either whether the black player (isTestForBlackPlayer == true) is in check,
 * or the white player (isTestForBlackPlayer == false) is in check.
 */
bool isCheck(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer)
{
	Position kingPos = getKingPosition(board, isTestForBlackPlayer);
	return isKingUnderCheck(board, isTestForBlackPlayer, &kingPos);
}

/** Returns if the black player (isTestForBlackPlayer-true) or white player (isTestForBlackPlayer=false)
 *	are in Matt (their king is in danger and cannot be saved).
 *	To optimize, this method accepts possibleMoves for the given player, to avoid calculating them all over again.
 */
bool isMatt(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, LinkedList* possibleMoves)
{
	return ((possibleMoves->length == 0) && (isCheck(board, isTestForBlackPlayer)));
}

/** Returns if the black player (isTestForBlackPlayer-true) or white player (isTestForBlackPlayer=false)
*	are in tie (their king is not in danger but no additional moves can be made).
*	To optimize, this method accepts possibleMoves for the given player, to avoid calculating them all over again.
*/
bool isTie(char board[BOARD_SIZE][BOARD_SIZE], bool isTestForBlackPlayer, LinkedList* possibleMoves)
{
	return ((possibleMoves->length == 0) && (!isCheck(board, isTestForBlackPlayer)));
}

/*
* Get all possible moves for the given square.
* If the square is illegal or vacant, empty list is returned.
* Otherwise we query the square to find out which player occupies the square, and returns the possible moves for that
* player's piece.
* Input:
*		board ~ The game board.
*		x, y ~ The position on board to search for moves.
*/
LinkedList* getMovesForSquare(char board[BOARD_SIZE][BOARD_SIZE], int x, int y)
{
	LinkedList* possibleMoves = createList(deleteMove);  // <-- This list contains the results of moves available.
														 // Note we change this list in the following service functions.
	if (g_memError)
		return NULL;

	if (!isSquareOnBoard(x, y) || isSquareVacant(board, x, y))
		return possibleMoves;

	bool isMovesForBlackPlayer = isSquareOccupiedByBlackPlayer(board, x, y);

	Position kingPos = getKingPosition(board, isMovesForBlackPlayer); // Position of current player's king
	Position startPos;
	startPos.x = x;
	startPos.y = y;
	getPieceMove(board, possibleMoves, isMovesForBlackPlayer, &startPos, &kingPos);

	if (g_memError)
	{
		deleteList(possibleMoves);
		return NULL;
	}

	return possibleMoves;
}