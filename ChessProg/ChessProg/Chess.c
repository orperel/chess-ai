#include "Chess.h"
#include <stdio.h>
#include "Types.h"
#include "Console.h"
#include "ChessMainWindow.h"

int main(int argc, char *argv[])
{
	// Determine console mode or gui mode
	bool isGuiMode = true;
	if (argc > 2) {
		printf("Usage: chessprog [console | gui] or chessprog only (default mode - console)\n");
		return 0;
	}
	if (argc == 2)
	{
		if (0 == strcmp(argv[1], GUI_MODE))
		{
			isGuiMode = true;
		}
		else if (0 != strcmp(argv[1], CONSOLE_MODE))
		{
			printf("Illegal argument\n");
			return 0;
		}
	}

	if (!isGuiMode)
	{
		return initConsoleMainLoop();
	}
	else
	{
		return runGuiMainLoop();
	}
}