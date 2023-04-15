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
	board->PvTable->pTable = NULL;
	//InitPvTable(board->PvTable);

	ParseFen(START_FEN, board);

	char input[6];
	int Move = NOMOVE;
	int PvNum = 0;
	int Max = 0;




	while (TRUE) {
		PrintBoard(board);
		printf("Enter a move > ");
		fgets(input, 6, stdin);

		if (input[0] == 'q') {
			break;
		}
		else if (input[0] == 'p') {
			Max = GetPvLine(4, board);
			printf("PvLine of %d Moves: ", Max);
			for (PvNum = 0; PvNum < Max; ++PvNum) {
				Move = board->pvArray[PvNum];
				printf(" %s", PrMove(Move));
			}
			printf("\n");
			//PerfTest(5, board);
		}
		else if (input[0] == 't') {
			TakeMove(board);
		}
		else {
			Move = ParseMove(input, board);
			if (Move != NOMOVE) {
				StorePvMove(board, Move);
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



