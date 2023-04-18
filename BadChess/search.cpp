// search.cpp

#include "stdio.h"
#include "defs.h"
#include <iostream>
#include <algorithm>


// Check if the time is up or interrupt from GUI
static void CheckUp(S_SEARCHINFO *info) {
	
	if (info->timeset == TRUE && GetTimeMs() > info->stoptime) {
		info->stopped = TRUE;
	}

	ReadInput(info); // Check if input is waiting
}


// Find the best score in the remaning moves, move ordering
static void PickNextMove(int moveNum, S_MOVELIST* list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = -INF_BOUND;
	int bestNum = moveNum;

	// Find best score
	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}
	// Reorder movelist
	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
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

	pos->HashTable->overWrite = 0;
	pos->HashTable->hit = 0;
	pos->HashTable->cut = 0;
	pos->ply = 0; // Set search ply to 0

	// Reset search info
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;

}


// Prevent horizon effect with Quiescence search
static int Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info) {
	
	// Valid board
	ASSERT(CheckBoard(pos));

	// Check time elapsed
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}


	++info->nodes; // Increment nodes

	// Repetition
	if (IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}
	// Max Depth
	if (pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	int Score = EvalPosition(pos);
	// Standing pat
	if (Score >= beta) {
		return beta;
	}
	if (Score > alpha) {
		alpha = Score;
	}

	// Generate captures
	S_MOVELIST list[1];
	GenerateAllCaptures(pos, list);

	// Variable definitions
	int MoveNum = 0;
	int Legal = 0;

	Score = -INF_BOUND;
	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// Move ordering
		PickNextMove(MoveNum, list);

		// Not legal move
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}

		++Legal;
		Score = -Quiescence(-beta, -alpha, pos, info); // Recursively call function
		TakeMove(pos);

		// Check status
		if (info->stopped == TRUE) {
			return 0;
		}

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
		}
	}
	return alpha;
}


// NegaMax Alpha Beta pruning search
static int AlphaBeta(int alpha, int beta, int depth, S_BOARD* pos, S_SEARCHINFO* info, int DoNull) {


	// Valid board
	ASSERT(CheckBoard(pos));
	ASSERT(beta > alpha);
	ASSERT(depth >= 0);
	// At terminal node
	if (depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	// Check time elapsed
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}

	++info->nodes;

	// Check for draw or repetition
	if ((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	// Search further if in check
	int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);
	if (InCheck == TRUE) {
		++depth;
	}

	int Score = -INF_BOUND;
	int PvMove = NOMOVE;


	// Probe hash entry
	if (ProbeHashEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE) {
		++pos->HashTable->cut;
		return Score;
	}

	// Null move
	if (DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 1) && depth >= 4) {
		MakeNullMove(pos);
		Score = -AlphaBeta(-beta, -beta + 1, depth - 4, pos, info, FALSE);
		TakeNullMove(pos);
		if (info->stopped == TRUE) {
			return 0;
		}

		if (Score >= beta && abs(Score) < ISMATE) {
			++info->nullCut;
			return beta;
		}
	}

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	// Variable definitions
	int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INF_BOUND;
	Score = -INF_BOUND;

	// Prioritize Principal variation move
	if (PvMove != NOMOVE) {
		for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if (list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// Move ordering
		PickNextMove(MoveNum, list);

		// Not legal move
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}

		++Legal;

		// TRYING Late Move Reduction
		if (MoveNum >= 5 and depth > 2 and not InCheck and !(list->moves[MoveNum].move & MFLAGCAP)) {
			Score = -AlphaBeta(-beta, -alpha, depth - 2, pos, info, TRUE);
			if (alpha < Score) {
				Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE);
			}
		}
		else {
			Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE);
		}


		//Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE); // Recursively call function
		TakeMove(pos);

		// Check status
		if (info->stopped == TRUE) {
			return 0;
		}

		// New best score
		if (Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			// New alpha score
			if (Score > alpha) {
				if (Score >= beta) {
					if (Legal == 1) {
						++info->fhf; // Searched the best move first
					}
					++info->fh;

					// Check for capture, adjust killers
					if (!(list->moves[MoveNum].move & MFLAGCAP)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}
					
					StoreHashEntry(pos, BestMove, beta, HFBETA, depth); // Store hash entry of position

					return beta;
				}
				alpha = Score;

				// Check for capture, adjust Hueristic
				if (!(list->moves[MoveNum].move & MFLAGCAP)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth; // Prioritizes moves closer to root
				}
			}
		}
	}

	// Checkmate and stalemate
	if (Legal == 0) {
		if (InCheck) {
			return -INF_BOUND + pos->ply; // Checkmate
		}
		else {
			return 0; // Stalemate
		}
	}

	// Store best move for new alpha value
	if (alpha != OldAlpha) {
		StoreHashEntry(pos, BestMove, BestScore, HFEXACT, depth); // Store hash entry of position
	}
	else {
		StoreHashEntry(pos, BestMove, alpha, HFALPHA, depth); // Store hash entry of position
	}

	return alpha;
}


// Iterative deepening from depth = 1 to MAXDEPTH
void SearchPosition(S_BOARD* pos, S_SEARCHINFO* info) {

	// Define variables
	int bestMove = NOMOVE;
	int bestScore = -INF_BOUND;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, info); // Prepare for search

	if (EngineOptions->UseBook == TRUE) {
		bestMove = GetBookMove(pos);
	}

	// Iterative deepening
	if (bestMove == NOMOVE) {
		for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {

			// Start alpha beta search
			bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, info, TRUE);

			// Check status
			if (info->stopped == TRUE) {
				break;
			}


			pvMoves = GetPvLine(currentDepth, pos); // Get PV line
			bestMove = pos->pvArray[0];

			if (info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {
				std::cout << "info score cp " << bestScore << " depth " << currentDepth << " nodes " << info->nodes <<
					" time " << GetTimeMs() - info->starttime << " pv ";
				for (int count = 0; count < pvMoves; ++count) {
					// print PV move
					std::cout << PrMove(pos->pvArray[count]);
					printf(" ");
				}
				std::cout << std::endl;

			}


		}
	}
	if (info->GAME_MODE == UCIMODE) {
		printf("bestmove %s\n", PrMove(bestMove));
	}
	else {
		printf("\n\n***!! BadChess makes move %s !!***\n\n", PrMove(bestMove));
		MakeMove(pos, bestMove);
		PrintBoard(pos);
	}

}



