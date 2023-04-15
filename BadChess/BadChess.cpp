// BadChess.cpp : Defines the entry point for the application.
//
#include "stdio.h"
#include "stdlib.h"
#include "defs.h"

#define CASTLE2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main()
{
	AllInit();

	S_BOARD board[1];

	ParseFen(CASTLE2, board);
	PrintBoard(board);

	S_MOVELIST list[1];
	GenerateAllMoves(board, list);

	PrintMoveList(list);

	return 0;
};