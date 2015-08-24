#ifndef GAME_MANAGER_
#define GAME_MANAGER_

#include <stdio.h>
#include "Types.h"

/* A constructor function for Position structs. */
Position* createPosition();

/* A constructor function for Move structs. */
Move* createMove(Position startPos);

/* A deep copy constructor function for Move structs. */
Move* cloneMove(Move* original);

/* A destructor function for Move structs */
void deleteMove(void* move);

#endif