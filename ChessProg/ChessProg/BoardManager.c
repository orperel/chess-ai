#include <stdlib.h>
#include "LinkedList.h"
#include "Chess.h"
#include "BoardManager.h"

void init_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			if ((i + j) % 2 == 0){
				if (j <= 3){
					board[i][j] = WHITE_P;
				}
				else if (j >= 6){
					board[i][j] = BLACK_P;
				}
				else{
					board[i][j] = EMPTY;
				}
			}
			else{
				board[i][j] = EMPTY;
			}
		}
	}
}

void print_line(){
	int i;
	printf("  |");
	for (i = 1; i < BOARD_SIZE * 4; i++){
		printf("-");
	}
	printf("|\n");
}

void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	print_line();
	for (j = BOARD_SIZE - 1; j >= 0; j--)
	{
		printf((j < 9 ? " %d" : "%d"), j + 1);
		for (i = 0; i < BOARD_SIZE; i++){
			printf("| %c ", board[i][j]);
		}
		printf("|\n");
		print_line();
	}
	printf("   ");
	for (j = 0; j < BOARD_SIZE; j++){
		printf(" %c  ", (char)('a' + j));
	}
	printf("\n");
}

/* Clear the Chess board (remove all the pieces). */
void clearBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			board[i][j] = EMPTY;
		}
	}
}

/* A constructor function for Position structs. */
Position* createPosition(int x, int y)
{
	Position* newPos = (Position*)malloc(sizeof(Position));
	if (newPos == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	newPos->x = x;
	newPos->y = y;

	return newPos;
}

/* A constructor function for Move structs. */
Move* createMove(Position startPos)
{
	Move* newMove = (Move*)malloc(sizeof(Move));
	if (newMove == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	newMove->initPos.x = startPos.x;
	newMove->initPos.y = startPos.y;
	newMove->nextPoses = createList(NULL);

	if (g_memError)
		return NULL;

	return newMove;
}

/* A deep copy constructor function for Move structs. */
Move* cloneMove(Move* original)
{
	Move* clone = createMove(original->initPos);
	if (g_memError)
		return NULL;

	Node* currNode = original->nextPoses->head;

	while (currNode != NULL)
	{
		Position* origPos = (Position*)currNode->data;
		Position* pos = createPosition(origPos->x, origPos->y);
		insertLast(clone->nextPoses, pos);

		currNode = currNode->next;
	}

	if (g_memError)
	{
		deleteMove((void*)clone);
		return NULL;
	}

	return clone;
}

/* A destructor function for Move structs. */
void deleteMove(void* move)
{
	deleteList(((Move*)move)->nextPoses);
	free(move);
}

/* A constructor function for GameStep structs. */
GameStep* createGameStep(char board[BOARD_SIZE][BOARD_SIZE], Move* move)
{
	GameStep* step = (GameStep*)malloc(sizeof(GameStep));
	if (step == NULL)
	{
		perror("Error: standard function malloc has failed");
		g_memError = true;
		return NULL;
	}

	char soldier = board[move->initPos.x][move->initPos.y];
	bool isBlackPlayer = (soldier == BLACK_P) || (soldier == BLACK_K);
	step->startPos = move->initPos;
	step->endPos = *((Position*)move->nextPoses->tail->data);
	step->currSoldier = board[step->startPos.x][step->startPos.y];

	step->isBecomeKing = isBecomeKing(step->currSoldier, step->endPos);

	step->removedPositions = createList(NULL);
	if (g_memError)
	{
		free(step);
	}

	step->removedTypes = NULL;

	// Check in which direction we advance
	Position* nextPos = (Position*)move->nextPoses->head->data;
	int deltaX = nextPos->x - step->startPos.x > 0 ? 1 : -1;
	int deltaY = nextPos->y - step->startPos.y > 0 ? 1 : -1;

	int currX = step->startPos.x + deltaX;
	int currY = step->startPos.y + deltaY;
	int enemyIndex = 0; // Index for enemy types we eat

	// We advance along the first diagonal since the king can eat over a large distance
	while ((currX != nextPos->x) && (currY != nextPos->y))
	{
		if (isSquareOccupiedByEnemy(board, isBlackPlayer, currX, currY))
		{
			// First eat - allocate types here.
			// We know how many soldiers were eaten by looking at nextPoses so we can pre-allocate.
			int numOfEatenSoldiers = move->nextPoses->length;
			char* allocatedTypes = (char*)malloc(sizeof(char) * numOfEatenSoldiers);

			if (allocatedTypes == NULL)
			{
				perror("Error: standard function malloc has failed");
				g_memError = true;
				deleteGameStep(step);
				return NULL;
			}

			step->removedTypes = allocatedTypes;

			// Add an enemy to the list of eaten soldiers
			Position* dNextPos = createPosition(currX, currY);
			if (g_memError)
			{
				free(dNextPos);
				deleteGameStep(step);
				return NULL;
			}

			insertLast(step->removedPositions, dNextPos);
			if (g_memError)
			{
				deleteGameStep(step);
				return NULL;
			}

			step->removedTypes[enemyIndex] = board[currX][currY];
			enemyIndex++;
		}

		currX += deltaX;
		currY += deltaY;
	}

	Node* nextPosNode = move->nextPoses->head->next;
	Position* currStart;

	// Aggregate the next eats if we have any.
	// If the current step is only a move or there are no more eats, this loop will not occur.
	while (NULL != nextPosNode)
	{
		currStart = nextPos;
		nextPos = (Position*)nextPosNode->data;
		deltaX = nextPos->x - currStart->x > 0 ? 1 : -1;
		deltaY = nextPos->y - currStart->y > 0 ? 1 : -1;

		currX += deltaX;
		currY += deltaY;

		// Add an enemy to the list of eaten soldiers
		Position* eatenPos = createPosition(currX, currY);
		if (g_memError)
		{
			deleteGameStep(step);
			return NULL;
		}

		insertLast(step->removedPositions, eatenPos);
		if (g_memError)
		{
			free(eatenPos);
			deleteGameStep(step);
			return NULL;
		}

		step->removedTypes[enemyIndex] = board[currX][currY];
		enemyIndex++;

		currX += deltaX;
		currY += deltaY;

		nextPosNode = nextPosNode->next;
	}

	return step;
}

/* A destructor function for GameStep structs. */
void deleteGameStep(GameStep* step)
{
	deleteList(step->removedPositions);
	free(step->removedTypes);
	free(step);
}

/* Execute game step on the board. */
void doStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step)
{
	// Remove start position
	board[step->startPos.x][step->startPos.y] = EMPTY;

	// Set end position
	if (step->isBecomeKing)
	{
		if (step->currSoldier == WHITE_P)
		{
			board[step->endPos.x][step->endPos.y] = WHITE_K;
		}
		else
		{
			board[step->endPos.x][step->endPos.y] = BLACK_K;
		}
	}
	else
	{
		board[step->endPos.x][step->endPos.y] = step->currSoldier;
	}

	// Remove all removed positions
	Node* currPos = step->removedPositions->head;
	int currX, currY;
	while (currPos != NULL)
	{
		currX = ((Position*)(currPos->data))->x;
		currY = ((Position*)(currPos->data))->y;
		board[currX][currY] = EMPTY;

		currPos = currPos->next;
	}
}

/* Undo game step on the board. */
void undoStep(char board[BOARD_SIZE][BOARD_SIZE], GameStep* step)
{
	// Remove end position
	board[step->endPos.x][step->endPos.y] = EMPTY;

	// Set start position
	board[step->startPos.x][step->startPos.y] = step->currSoldier;

	// Restore all removed positions
	Node* currPos = step->removedPositions->head;
	int i = 0;
	int currX, currY;
	while (currPos != NULL)
	{
		currX = ((Position*)(currPos->data))->x;
		currY = ((Position*)(currPos->data))->y;
		board[currX][currY] = step->removedTypes[i];

		currPos = currPos->next;
		i++;
	}
}

/* Returns if the square is on the board area. */
bool isSquareOnBoard(int i, int j)
{
	// Check we're in the board index range and we're on a black square
	return (i >= 0) && (j >= 0) && (i < BOARD_SIZE) && (j < BOARD_SIZE) && (0 == ((i + j) % 2));
}

/* Returns if the square is on the board and has no soldiers on it. */
bool isSquareVacant(char board[BOARD_SIZE][BOARD_SIZE], int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (board[i][j] == EMPTY);
}

/* Returns if the square is on the board and occupied by the current player. */
bool isSquareOccupiedByCurrPlayer(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (!isMovesForBlackPlayer && ((board[i][j] == WHITE_K) || (board[i][j] == WHITE_P))) ||
		(isMovesForBlackPlayer && ((board[i][j] == BLACK_K) || (board[i][j] == BLACK_P)));
}

/* Returns if the square is on the board and occupied by the enemy. */
bool isSquareOccupiedByEnemy(char board[BOARD_SIZE][BOARD_SIZE], bool isMovesForBlackPlayer, int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return (isMovesForBlackPlayer && ((board[i][j] == WHITE_K) || (board[i][j] == WHITE_P))) ||
		(!isMovesForBlackPlayer && ((board[i][j] == BLACK_K) || (board[i][j] == BLACK_P)));
}

/* Returns if the square is on the board and occupied any king. */
bool isSquareOccupiedByKing(char board[BOARD_SIZE][BOARD_SIZE], int i, int j)
{
	if (!isSquareOnBoard(i, j))
		return false;

	return ((board[i][j] == WHITE_K) || (board[i][j] == BLACK_K));
}

/* Returns if the soldier goes to endPos, will it become a king. */
bool isBecomeKing(char soldier, Position endPos)
{
	return ((soldier == BLACK_P) && (endPos.y == 0)) ||
		((soldier == WHITE_P) && (endPos.y == BOARD_SIZE - 1));
}

/*
* Returns the size of the army of the black / white player (according to the input isBlackSoldiers parameter).
* Results detail how many kings and men remain.
*/
Army getArmy(char board[BOARD_SIZE][BOARD_SIZE], bool isBlackSoldiers)
{
	Army army = { 0 };

	int i, j; // i = column, j = row

	for (j = 0; j < BOARD_SIZE; j++)
	{
		for (i = 0; i < BOARD_SIZE; i++)
		{
			char soldier = board[j][i];

			if (isBlackSoldiers)
			{
				switch (soldier)
				{
				case BLACK_P:
					army.pawns++;
					break;
				case BLACK_B:
					army.bishops++;
					break;
				case BLACK_R:
					army.rooks++;
					break;
				case BLACK_N:
					army.knights++;
					break;
				case BLACK_Q:
					army.queens++;
					break;
				case BLACK_K:
					army.kings++;
					break;
				}
			}
			else
			{
				switch (soldier)
				{
				case WHITE_P:
					army.pawns++;
					break;
				case WHITE_B:
					army.bishops++;
					break;
				case WHITE_R:
					army.rooks++;
					break;
				case WHITE_N:
					army.knights++;
					break;
				case WHITE_Q:
					army.queens++;
					break;
				case WHITE_K:
					army.kings++;
					break;
				}
			}
		}
	}

	return army;
}

/* Validate that there are no men in the opponent edge. */
bool validEdges(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i;
	for (i = 0; i < BOARD_SIZE; i += 2)
	{
		if (board[BOARD_SIZE - 1][i] == BLACK_P)
		{	// Bottom edge
			return false;
		}

		if (board[0][i + 1] == WHITE_P)
		{	// Top edge
			return false;
		}
	}

	return true;
}