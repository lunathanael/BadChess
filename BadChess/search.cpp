// search.cpp

#include "stdio.h"
#include "defs.h"



// Check if the time is up or interrupt from GUI
static void CheckUp() {

}

// Search for a position repetition
static int IsRepetition(const S_BOARD* pos) {
	int index = 0;

	// Starting from last fiftymove update
	for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; ++index) {
		
		// Index outside of bounds
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		
		if (pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}

	return FALSE;
}


// Clear search information for a new search
static void ClearForSearch(S_BOARD* pos, S_SEARCHINFO* info) {
	
	// Define indices
	int index = 0;
	int index2 = 0;

	// Clear search hisotry
	for (index = 0; index < 13; ++index) {
		for (index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	// Clear killer moves
	for (index = 0; index < 2; ++index) {
		for (index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	// Clear prinicipal variation table
	ClearPvTable(pos->PvTable);
	pos->ply = 0; // Set search ply to 0

	// Reset search info
	info->starttime = GetTimeMs();
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;

}


// Prevent horizon effect with Quiescence search
static int Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info) {
	return 0;
}


// NegaMax Alpha Beta pruning search
static int AlphaBeta(int alpha, int beta, int depth, S_BOARD* pos, S_SEARCHINFO* info, int DoNull) {

	// Valid board
	ASSERT(CheckBoard(pos));

	// At terminal node
	if (depth == 0) {
		++info->nodes;
		return EvalPosition(pos);
	}

	++info->nodes;

	// Check for draw or repetition
	if (IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}


	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	// Variable definitions
	int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	int Score = -INFINITE;

	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// Not legal move
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}

		++Legal;
		Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE); // Recursively call function
		TakeMove(pos);

		// New alpha score
		if (Score > alpha) {
			if (Score >= beta) {
				if (Legal == 1) {
					++info->fhf; // Searched the best move first
				}
				++info->fh;
				return beta;
			}
			alpha = Score;
			BestMove = list->moves[MoveNum].move;
		}
	}

	// Checkmate and stalemate
	if (Legal == 0) {
		if (SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos)) {
			return -INFINITE + pos->ply; // Checkmate
		}
		else {
			return 0; // Stalemate
		}
	}

	// Store best move for new alpha value
	if (alpha != OldAlpha) {
		StorePvMove(pos, BestMove);
	}

	return alpha;
}


// Iterative deepening from depth = 1 to MAXDEPTH
void SearchPosition(S_BOARD* pos, S_SEARCHINFO* info) {

	// Define variables
	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, info); // Prepare for search

	// Iterative deepening
	for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {

		// Start alpha beta search
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);

		// Out of time

		pvMoves = GetPvLine(currentDepth, pos); // Get PV line
		bestMove = pos->pvArray[0];

		printf("Depth:%d score:%d move:%s nodes:%ld ", currentDepth, bestScore, PrMove(bestMove), info->nodes);

		pvMoves = GetPvLine(currentDepth, pos);
		printf("PV:");
		for (pvNum = 0; pvNum < pvMoves; ++pvNum) {
			printf(" %s", PrMove(pos->pvArray[pvNum]));
		}
		printf("\n");
		printf("Ordering:%.2f\n", (info->fhf / info->fh));
	}



}



