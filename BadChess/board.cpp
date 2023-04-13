// board.cpp

#include "stdio.h"
#include "defs.h"

// Reset the board
void ResetBoard(S_BOARD *pos) {
	
	int index = 0;

	// Set all squares to "offboard"
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		pos->pieces[index] = OFFBOARD;
	}
}