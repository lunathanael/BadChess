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
		score -= PawnTable[MIRROR64(SQ64(sq))];
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
		score -= PawnTable[MIRROR64(SQ64(sq))];
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
		score -= PawnTable[MIRROR64(SQ64(sq))];
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
		score -= PawnTable[MIRROR64(SQ64(sq))];
	}

	// Negate score for negamax search
	if (pos->side == WHITE) {
		return score;
	}
	else {
		return -score;
	}
}

