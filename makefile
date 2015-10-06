all: chessprog

clean:
	-rm Chess.o Types.o Console.o LinkedList.o BoardManager.o GameCommands.o GameLogic.o Minimax.o chessprog

chessprog: Chess.o Types.o Console.o LinkedList.o BoardManager.o GameCommands.o GameLogic.o Minimax.o
	gcc -o chessprog Chess.o Types.o Console.o LinkedList.o BoardManager.o GameCommands.o GameLogic.o Minimax.o -lm -std=c99 -pedantic-errors -g

Chess.o: Chess.h Types.h Console.h Chess.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm Chess.c

Types.o: Types.h Types.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm Types.c

Console.o: Console.h Types.h LinkedList.h BoardManager.h GameCommands.h Console.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm Console.c

LinkedList.o: LinkedList.h LinkedList.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm LinkedList.c

BoardManager.o: Types.h BoardManager.h LinkedList.h BoardManager.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm BoardManager.c

GameCommands.o: Types.h LinkedList.h BoardManager.h GameCommands.h GameLogic.h Minimax.h GameCommands.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm GameCommands.c

GameLogic.o: LinkedList.h Types.h BoardManager.h GameLogic.h GameLogic.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm GameLogic.c

Minimax.o: Types.h Minimax.h BoardManager.h LinkedList.h GameLogic.h Minimax.c
	gcc -std=c99 -pedantic-errors -c -Wall -g -lm Minimax.c