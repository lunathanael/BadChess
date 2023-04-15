// BadChess.cpp : Defines the entry point for the application.
//
#include "stdio.h"
#include "stdlib.h"
#include "defs.h"




int main()
{
	AllInit();

	S_BOARD board[1];
	S_MOVELIST list[1];

	ParseFen(START_FEN, board);

	char input[6];
	int Move = NOMOVE;

	while (TRUE) {
		PrintBoard(board);
		printf("Enter a move > ");
		fgets(input, 6, stdin);

		if (input[0] == 'q') {
			break;
		}
		else if (input[0] == 'p') {
			PerfTest(5, board);
		}
		else if (input[0] == 't') {
			TakeMove(board);
		}
		else {
			Move = ParseMove(input, board);
			if (Move != NOMOVE) {
				MakeMove(board, Move);
				//if (IsRepetition(board)) {
				//	printf("REP SEEN\n");
				//}
			}
			else {
				printf("Move Not Parsed:%s\n",input); 
			}
		}

		fflush(stdin);
	}

	return 0;
};



