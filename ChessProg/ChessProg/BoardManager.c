#include "LinkedList.h"
#include "Chess.h"
#include "BoardManager.h"

/* A constructor function for Position structs. */
Position* createPosition(int x, int y)
{
	Position* newPos = (Position*)malloc(sizeof(Position));
	if (newPos == NULL)
	{
		perror_message("malloc");
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
		perror_message("malloc");
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