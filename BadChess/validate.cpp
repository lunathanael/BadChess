// validate.cpp

#include "defs.h"

// Check if square is on the board
int SqOnBoard(const int sq) {
	return FilesBrd[sq] == OFFBOARD ? 0 : 1;
}

// Check if the side is valid
int SideValid(const int side) {
	return (side == WHITE || side == BLACK) ? 1 : 0;
}

// Check if the File and Rank is valid
int FileRankValid(const int fr) {
	return (fr >= 0 && fr <= 7) ? 1 : 0;
}

// Check if the piece value is valid including EMPTY
int PieceValidEmpty(const int pce) {
	return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}

// Check if the piece valid excluding EMPTY
int PieceValid(const int pce) {
	return (pce >= wP && pce <= bK) ? 1 : 0;
}