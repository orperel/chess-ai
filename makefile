O_FILES = Chess.o Types.o Console.o ChessMainWindow.o LinkedList.o BoardManager.o GameCommands.o GuiFW.o ChessGuiPlayerSelectWindow.o ChessGuiCommons.o ChessGuiGameWindow.o GameLogic.o Minimax.o ChessGuiGameControl.o ChessGuiAISettingsWindow.o

CFLAGS = -std=c99 -pedantic-errors -c -Wall -g -lm `sdl-config --cflags`


all: chessprog

clean:
	-rm $(O_FILES) chessprog

chessprog: $(O_FILES)
	gcc -o chessprog $(O_FILES) -lm -std=c99 -pedantic-errors -g `sdl-config --libs`

Chess.o: Chess.h Types.h Console.h Chess.c
	gcc $(CFLAGS) Chess.c

Types.o: Types.h Types.c
	gcc $(CFLAGS) Types.c

Console.o: Console.h Types.h LinkedList.h BoardManager.h GameCommands.h Console.c
	gcc $(CFLAGS) Console.c

ChessMainWindow.o: GuiFW.h ChessMainWindow.h BoardManager.h ChessGuiPlayerSelectWindow.h ChessGuiCommons.h GameCommands.h ChessGuiGameWindow.h ChessMainWindow.c
	gcc $(CFLAGS) ChessMainWindow.c

LinkedList.o: LinkedList.h LinkedList.c
	gcc $(CFLAGS) LinkedList.c

BoardManager.o: Types.h BoardManager.h LinkedList.h BoardManager.c
	gcc $(CFLAGS) BoardManager.c

GameCommands.o: Types.h LinkedList.h BoardManager.h GameCommands.h GameLogic.h Minimax.h GameCommands.c
	gcc $(CFLAGS) GameCommands.c

GuiFW.o: GuiFW.h GuiFW.c
	gcc $(CFLAGS) GuiFW.c

ChessGuiPlayerSelectWindow.o: GuiFW.h Types.h ChessGuiPlayerSelectWindow.h ChessGuiGameControl.h ChessGuiCommons.h GameCommands.h ChessGuiGameWindow.h ChessMainWindow.h ChessGuiAISettingsWindow.h ChessGuiPlayerSelectWindow.c
	gcc $(CFLAGS) ChessGuiPlayerSelectWindow.c

ChessGuiCommons.o: GuiFW.h ChessGuiCommons.h Types.h GameCommands.h ChessGuiCommons.c
	gcc $(CFLAGS) ChessGuiCommons.c

ChessGuiGameWindow.o: GuiFW.h Types.h ChessGuiGameWindow.h ChessGuiGameControl.h BoardManager.h GameCommands.h ChessGuiCommons.h ChessMainWindow.h ChessGuiGameWindow.c
	gcc $(CFLAGS) ChessGuiGameWindow.c

GameLogic.o: LinkedList.h Types.h BoardManager.h GameLogic.h GameLogic.c
	gcc $(CFLAGS) GameLogic.c

Minimax.o: Types.h Minimax.h BoardManager.h LinkedList.h GameLogic.h Minimax.c
	gcc $(CFLAGS) Minimax.c

ChessGuiGameControl.o: GuiFW.h Types.h ChessGuiGameControl.h ChessGuiGameControl.c
	gcc $(CFLAGS) ChessGuiGameControl.c

ChessGuiAISettingsWindow.o: GuiFW.h Types.h ChessGuiAISettingsWindow.h ChessGuiGameControl.h ChessGuiCommons.h GameCommands.h ChessGuiGameWindow.h ChessMainWindow.h ChessGuiAISettingsWindow.c
	gcc $(CFLAGS) ChessGuiAISettingsWindow.c