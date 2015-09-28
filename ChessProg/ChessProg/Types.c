#include "Types.h"

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