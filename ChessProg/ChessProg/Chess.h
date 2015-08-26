#ifndef CHESS_
#define CHESS_

#include <stdio.h>
#include "Types.h"

#define WHITE_P 'm'
#define WHITE_B 'b'
#define WHITE_N 'n'
#define WHITE_R 'r'
#define WHITE_Q 'q'
#define WHITE_K 'k'

#define BLACK_P 'M'
#define BLACK_B 'B'
#define BLACK_N 'N'
#define BLACK_R 'R'
#define BLACK_Q 'Q'
#define BLACK_K 'K'

#define EMPTY ' '

#define BOARD_SIZE 8
#define MAX_ARGS 5			// Max number of args supported by shell
#define LINE_LENGTH 50		// Shell "buffer" size used to read user input
#define MAX_DEPTH 4			// Maximum depth miniMax algorithm depth can be
#define MAX_SOLDIERS 16		// Maximum number of soldiers per player

#define DIFFICULTY_DEPTH_COMMAND "difficulty depth"
#define USER_COLOR_COMMAND "user_color"
#define CLEAR_COMMAND "clear"
#define REMOVE_COMMAND "rm"
#define SET_COMMAND "set"
#define PRINT_COMMAND "print"
#define QUIT_COMMAND "quit"
#define START_COMMAND "start"
#define MOVE_COMMAND "move"
#define GETMOVES_COMMAND "get_moves"

#define WELCOME_TO_CHESS "Welcome to Chess!\n"
#define ENTER_SETTINGS "Enter game settings:\n" 
#define WRONG_GAME_MODE "Wrong game mode\n"
#define TWO_PLAYERS_GAME_MODE "Running game in 2 players mode\n"
#define PLAYER_VS_AI_GAME_MODE "Running game in player vs. AI mode\n"
#define WRONG_MINIMAX_DEPTH "Wrong value for minimax depth. The value should be between 1 to 4\n"
#define WRONG_FILE_NAME "Wrong file name\n"
#define WRONG_POSITION "Invalid position on the board\n"
#define WRONG_SET "Setting this piece creates an invalid board\n"  
#define NO_PIECE "The specified position does not contain your piece\n"
#define WRONG_BOARD_INITIALIZATION "Wrong board initialization\n"
#define ENTER_YOUR_MOVE "Enter your move:\n" 
#define COMPUTER_MSG "Computer: move "
#define BLACK_STR "Black"
#define WHITE_STR "White"

#define ILLEGAL_COMMAND "Illegal command, please try again\n"
#define ILLEGAL_MOVE "Illegal move\n"
 
#define WRONG_ROOK_POSITION "Wrong position for a rook\n" 
#define ILLEGAL_CASTLING_MOVE "Illegal castling move\n"  

#define TIE "The game ends in a tie\n"
#define WIN_MSG "%s player wins!\n"

#define print_message(message) (printf("%s", message));

/* -- Globals Declaration -- */

/* Configuration for mini-max algorithm. Maximum depth of recursion. Default to 1. */
extern int g_minimaxDepth;

/* Is the user the black color (true) or white color (false). Default to White. */
extern bool g_isUserBlack;

/* A buffer allocated for reading user input. The user's command will not exceed 50 characters. */
extern char g_inputLine[LINE_LENGTH];

/* True if there was an allocation error somewhere in the program (would cause an exit). Else false. */
extern bool g_memError;

#endif CHESS_