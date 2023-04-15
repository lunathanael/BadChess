// io.cpp

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

// Print Square
char* PrSq(const int sq) {

	static char SqStr[3];

	int file = FilesBrd[sq];
	int rank = RanksBrd[sq];

	sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));

	return SqStr; // Return pointer to string
}

// Print Move
char* PrMove(const int move) {

	static char MvStr[6];

	int ff = FilesBrd[FROMSQ(move)];
	int rf = RanksBrd[FROMSQ(move)];
	int ft = FilesBrd[TOSQ(move)];
	int rt = RanksBrd[TOSQ(move)];

	int promoted = PROMOTED(move);

	if (promoted) {
		char pchar = 'q';
		if (IsKN(promoted)) {
			pchar = 'n';
		}
		else if (IsRQ(promoted) && !IsBQ(promoted)) {
			pchar = 'r';
		}
		else if (!IsRQ(promoted) && IsBQ(promoted)) {
			pchar = 'b';
		}
		sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
	}					
	else {
		sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
	}

	return MvStr; // Return pointer to array
}

// Print the move list
void PrintMoveList(const S_MOVELIST* list) {
	// Define the indices
	int index = 0;
	int score = 0;
	int move = 0;
	
	printf("MoveList:\n");

	// Print each move in movelist
	for (index = 0; index < list->count; ++index) {
		move = list->moves[index].move;
		score = list->moves[index].score;

		printf("Move:%d > %s (score:%d)\n", index + 1, PrMove(move), score);

	}
	printf("MoveList Total %d Moves:\n\n", list->count);

}