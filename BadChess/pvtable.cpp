// pvtable.cpp


#include "stdio.h"
#include "defs.h"



#define EXTRACT_SCORE(x) ((x & 0xFFFF) - INFINF)
#define EXTRACT_DEPTH(x) ((x>>16) & 0x3F)
#define EXTRACT_FLAGS(x) ((x>>23) & 0x3)
#define EXTRACT_MOVE(x) ((int)(x>>25))

#define FOLD_DATA(sc, de, fl, mv) ( (sc + INFINF) | (de << 16) | (fl<<23) | ((U64)mv << 25)) // Create data


void DataCheck(const int move) {
	int depth = rand() % MAXDEPTH;
	int flag = rand() % 3;
	int score = rand() % INFINF;

	U64 data FOLD_DATA(score, depth, flag, move);

	printf("Orig: move %s d%d fl%d sc%d data%llX\n", PrMove(move), depth, flag, score, data); 
	printf("Check: move %s d%d fl%d sc%d\n", PrMove(EXTRACT_MOVE(data)), EXTRACT_DEPTH(data), EXTRACT_FLAGS(data), EXTRACT_SCORE(data));
}


void TempHashTest(const std::string& command) {
	S_BOARD board[1];
	ParseFen(command, board);

	S_MOVELIST list[1];
	GenerateAllMoves(board, list);

	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		if (!MakeMove(board, list->moves[MoveNum].move)) {
			continue;
		}
		TakeMove(board);
		DataCheck(list->moves[MoveNum].move);
	}
	return;
}


void VerifyEntrySMP(S_HASHENTRY* entry) {
	U64 data = FOLD_DATA(entry->score, entry->depth, entry->flags, entry->move);
	U64 key = entry->posKey ^ data;

	if (data != entry->smp_data) { printf("data error"); exit(1); }
	if (key != entry->smp_key) { printf("key error"); exit(1); }

	int move = EXTRACT_MOVE(data);
	int flags = EXTRACT_FLAGS(data);
	int score = EXTRACT_SCORE(data);
	int depth = EXTRACT_DEPTH(data);

	if (flags != entry->flags) { printf("flag error"); exit(1); }
	if (score != entry->score) { printf("score error"); exit(1); }
	if (depth != entry->depth) { printf("score error"); exit(1); }
}



S_HASHTABLE HashTable[1];

// Fill PvLine array
int GetPvLine(const int depth, S_BOARD* pos, const S_HASHTABLE * table) {

	ASSERT(depth < MAXDEPTH && depth >= 1);

	int move = ProbePvMove(pos, table); // Prove pvTable for move
	int count = 0;

	while (move != NOMOVE && count < depth) {

		// Valid depth
		ASSERT(count < MAXDEPTH);

		if (MoveExists(pos, move)) {
			MakeMove(pos, move);
			pos->pvArray[count++] = move;
		}
		else {
			break; // Illegal move or count reached
		}
		move = ProbePvMove(pos, table);
	}

	// Undo Moves
	while (pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
}


// Function to clear the pvTable
void ClearHashTable(S_HASHTABLE* table) {

	S_HASHENTRY* tableEntry;

	for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; ++tableEntry) {
		tableEntry->posKey = 0ULL;
		tableEntry->move = NOMOVE;
		tableEntry->depth = 0;
		tableEntry->score = 0;
		tableEntry->flags = 0;
		tableEntry->age = 0;
		tableEntry->smp_data = 0ULL;
		tableEntry->smp_key = 0ULL;

	}
	table->currentAge = 0;
	table->newWrite = 0;
}



// Function to initialize pvTable
void InitHashTable(S_HASHTABLE* table, const int MB) {

	int HashSize = 0x100000 * MB;
	table->numEntries = HashSize / sizeof(S_HASHENTRY);
	table->numEntries -= 2;

	if (table->pTable != NULL) {
		free(table->pTable);
	}

	table->pTable = (S_HASHENTRY*)malloc(table->numEntries * sizeof(S_HASHENTRY));
	if (table->pTable == NULL) {
		printf("Hash Allocation Failed, trying %dMB...\n", MB / 2);
		InitHashTable(table, MB / 2);
	}
	else {
		ClearHashTable(table);
		printf("HashTable init complete with %d entries\n", table->numEntries);
	}
}


// Store move into pvTable
void StoreHashEntry(S_BOARD* pos, S_HASHTABLE* table, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % table->numEntries;

	// Valid index
	ASSERT(index >= 0 && index <= table->numEntries - 1);

	int replace = FALSE;

	if (table->pTable[index].posKey == 0) {
		++table->newWrite;
		replace = TRUE;
	}
	else {
		if ( (table->pTable[index].age < table->currentAge) || (table->pTable[index].depth <= depth) ) {
			replace = TRUE;
		}
		//++table->overWrite;
	}

	if (replace == FALSE) return;

	if (score > ISMATE)
		score -= pos->ply;
	else if (score < -ISMATE)
		score += pos->ply;

	// Create smp entry
	U64 smp_data = FOLD_DATA(score, depth, flags, move);
	U64 smp_key = pos->posKey ^ smp_data;

	table->pTable[index].move = move;
	table->pTable[index].posKey = pos->posKey;
	table->pTable[index].flags = flags;
	table->pTable[index].score = score;
	table->pTable[index].depth = depth;
	table->pTable[index].age = table->currentAge;
	table->pTable[index].smp_data = smp_data;
	table->pTable[index].smp_key = smp_key;

	// Check smp
	VerifyEntrySMP(&table->pTable[index]);
}


// Return move from posKey
int ProbePvMove(const S_BOARD* pos, const S_HASHTABLE* table) {

	int index = pos->posKey % table->numEntries;
	// Valid index
	ASSERT(index >= 0 && index <= table->numEntries - 1);
	// Check for matching key

	if (table->pTable[index].posKey == pos->posKey) {
		return table->pTable[index].move;
	}

	return NOMOVE;
}


int ProbeHashEntry(S_BOARD* pos, S_HASHTABLE* table, int* move, int* score, int alpha, int beta, int depth) {

	int index = pos->posKey % table->numEntries;

	ASSERT(index >= 0 && index <= table->numEntries - 1);
	ASSERT(depth >= 1 && depth < MAXDEPTH);
	ASSERT(alpha < beta);
	ASSERT(alpha >= -INF_BOUND && alpha <= INF_BOUND);
	ASSERT(beta >= -INF_BOUND && beta <= INF_BOUND);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

	if (table->pTable[index].posKey == pos->posKey) {

		// Verify Correct Information
		U64 test_key = table->pTable[index].posKey ^ table->pTable[index].smp_data;
		if (test_key != table->pTable[index].smp_key) printf("Error test_key\n");

		VerifyEntrySMP(&table->pTable[index]);

		*move = table->pTable[index].move;
		if (table->pTable[index].depth >= depth) {
			++table->hit;

			ASSERT(table->pTable[index].depth >= 1 && table->pTable[index].depth < MAXDEPTH);
			ASSERT(table->pTable[index].flags >= HFALPHA && table->pTable[index].flags <= HFEXACT);

			*score = table->pTable[index].score;
			if (*score > ISMATE) *score -= pos->ply;
			else if (*score < -ISMATE) *score += pos->ply;

			switch (table->pTable[index].flags) {

			case HFALPHA: if (*score <= alpha) {
				*score = alpha;
				return TRUE;
			}
						break;
			case HFBETA: if (*score >= beta) {
				*score = beta;
				return TRUE;
			}
					   break;
			case HFEXACT:
				return TRUE;
				break;
			default: ASSERT(FALSE); break;
			}
		}
	}

	return FALSE;
}


