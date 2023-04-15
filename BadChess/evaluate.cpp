// evaluate.cpp


#include "stdio.h"
#include "defs.h"


/* PIECE SQUARE TABLES in pawns/100 */

const int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};

const int KnightTable[64] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};

const int BishopTable[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};

const int RookTable[64] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0
};

const int MIRROR64[64] = {
	56, 57, 58, 59, 60, 61, 62, 63,
	48, 49, 50, 51, 52, 53, 54, 55,
	40, 41, 42, 43, 44, 45, 46, 47,
	32, 33, 34, 35, 36, 37, 38, 39,
	24, 25, 26, 27, 28, 29, 30, 31,
	16, 17, 18, 19, 20, 21, 22, 23,
	8,  9,  10, 11, 12, 13, 14, 15,
	0,  1,  2,  3,  4,  5,  6,  7
};

//const int KingE[64] = {
//	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
//	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
//	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
//	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
//	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
//	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
//	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
//	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50
//};
//
//const int KingO[64] = {
//	0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
//	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
//	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
//	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
//	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
//	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
//	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
//	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70
//};



// Return evaluation score from position
int EvalPosition(const S_BOARD* pos) {

	// Define indices
	int pce;
	int pceNum;
	int sq;

	// Material score
	int score = pos->material[WHITE] - pos->material[BLACK];

	/* PSQT SCORES */
	
	// Pawns
	pce = wP;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score += PawnTable[SQ64(sq)];
	}
	pce = bP;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score -= PawnTable[MIRROR64[SQ64(sq)]];
	}

	// Knights
	pce = wN;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score += PawnTable[SQ64(sq)];
	}
	pce = bN;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score -= PawnTable[MIRROR64[SQ64(sq)]];
	}

	// Bishops
	pce = wB;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score += PawnTable[SQ64(sq)];
	}
	pce = bB;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score -= PawnTable[MIRROR64[SQ64(sq)]];
	}

	// Rooks
	pce = wR;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score += PawnTable[SQ64(sq)];
	}
	pce = bR;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square
		score -= PawnTable[MIRROR64[SQ64(sq)]];
	}

	// Negate score for negamax search
	if (pos->side == WHITE) {
		return score;
	}
	else {
		return -score;
	}
}

