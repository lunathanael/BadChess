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
	int bestScore = -2147483645;
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



static inline void score_moves(S_BOARD* pos, S_SEARCHINFO* info, S_MOVELIST* move_list, int PvMove) {
	//Loop through all the move in the movelist
	for (int i = 0; i < move_list->count; ++i) {
		int move = move_list->moves[i].move;
		//If the move is from the TT (aka it's our hashmove) give it the highest score
		if (move == PvMove) {
			move_list->moves[i].score = INT32_MAX - 100;
			continue;
		}
		else if (move & MFLAGPROM) {
			move_list->moves[i].score = 2000000000 + PROMOTED(move);
			continue;
		}
		//if the move is an enpassant or a promotion give it a score that a good capture of type pawn-pawn would have
		else if (move & MFLAGEP) {
			move_list->moves[i].score = 105 + 900000000; // Good capture score
			continue;
		}
		//if the move is a capture sum the mvv-lva score to a variable that depends on whether the capture has a positive SEE or not 
		else if (move & MFLAGCAP) {
			move_list->moves[i].score =
				MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 900000000;// *SEE(pos, move, -107);
			continue;
		}
		//First  killer move always comes after the TT move,the promotions and the good captures and before anything else
		else if (pos->searchKillers[0][pos->ply] == move) {
			move_list->moves[i].score = 800000000; // Killer move 1
			continue;
		}
		//Second killer move always comes after the first one
		else if (pos->searchKillers[1][pos->ply] == move) {
			move_list->moves[i].score = 700000000; // Kilelr move 2
			continue;
		}
		////After the killer moves try the Counter moves
		//else if (move == info->CounterMoves[FROMSQ(ss->move)][TOSQ(ss->move)])
		//{
		//	move_list->moves[i].score = 600000000;
		//	continue;
		//}
		//if the move isn't in any of the previous categories score it according to the history heuristic
		else {
			move_list->moves[i].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
			//move_list->moves[i].score = GetHHScore(pos, sd, move) + 2 * GetCHScore(pos, sd, ss, move);
			continue;
		}
	}

	return;
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
static int Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info, int depth) {
	
	// Valid board
	ASSERT(CheckBoard(pos));

	// Check time elapsed
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}


	++info->nodes; // Increment nodes

	// Repetition
	if (IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 1 - (info->nodes & 2); // Prevent 3-fold blindness
	}

	// Max Depth
	if (pos->ply > MAXDEPTH - 1) {
		int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);
		return InCheck ? 0 : EvalPosition(pos);
	}

	int PvMove = NOMOVE;

	int Score = EvalPosition(pos);

	// Probe hash entry
	if (ProbeHashEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE) {
		++pos->HashTable->cut;
		return Score;
	}

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

	score_moves(pos, info, list, PvMove);

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
		Score = -Quiescence(-beta, -alpha, pos, info, depth - 1); // Recursively call function
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
		return Quiescence(alpha, beta, pos, info, depth - 1);
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

	score_moves(pos, info, list, PvMove);

	// Variable definitions
	int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INF_BOUND;
	Score = -INF_BOUND;

	//// Prioritize Principal variation move, CHANGE*
	//if (PvMove != NOMOVE) {
	//	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
	//		if (list->moves[MoveNum].move == PvMove) {
	//			list->moves[MoveNum].score = 2000000; 
	//			//printf("Pv move found \n");
	//			break;
	//		}
	//	}
	//}



	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// Move ordering
		PickNextMove(MoveNum, list);

		// Not legal move
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}

		++Legal;
		Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE); // Recursively call function
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




int AspirationWindowSearch(int prev_eval, int depth, S_BOARD* pos, S_SEARCHINFO* info) {
	int score = 0;

	//Search_stack stack[MAXDEPTH], * ss = stack;

	//We set an expected window for the score at the next search depth, this window is not 100% accurate so we might need to try a bigger window and re-search the position
	int delta = 12;
	// define initial alpha beta bounds
	int alpha = -INF_BOUND;
	int beta = INF_BOUND;

	// only set up the windows is the search depth is bigger or equal than Aspiration_Depth to avoid using windows when the search isn't accurate enough
	if (depth >= 3) {
		alpha = std::max(-INF_BOUND, prev_eval - delta);
		beta = std::min(prev_eval + delta, INF_BOUND);
	}

	//Stay at current depth if we fail high/low because of the aspiration windows
	while (true) {

		score = AlphaBeta(alpha, beta, depth, pos, info, TRUE);

		// stop calculating and return best move so far
		if (info->stopped) break;

		// we fell outside the window, so try again with a bigger window, if we still fail after we just search with a full window
		if ((score <= alpha)) {
			beta = (alpha + beta) / 2;
			alpha = std::max(-INF_BOUND, score - delta);
		}

		// we fell outside the window, so try again with a bigger window, if we still fail after we just search with a full window
		else if ((score >= beta)) {
			beta = std::min(score + delta, INF_BOUND);
		}
		else break;
		delta *= 1.44;
	}
	return score;
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
			//bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, info, TRUE);
			bestScore = AspirationWindowSearch(bestScore, currentDepth, pos, info);

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



