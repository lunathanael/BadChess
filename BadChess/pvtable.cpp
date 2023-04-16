// pvtable.cpp


#include "stdio.h"
#include "defs.h"


// Fill PvLine array
int GetPvLine(const int depth, S_BOARD* pos) {

	ASSERT(depth < MAXDEPTH && depth >= 1);

	int move = ProbePvMove(pos); // Prove pvTable for move
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
		move = ProbePvMove(pos);
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
	}
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
void StoreHashEntry(S_BOARD* pos, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % pos->HashTable->numEntries;

	// Valid index
	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);

	if (pos->HashTable->pTable[index].posKey == 0) {
		++pos->HashTable->newWrite;
	}
	else {
		++pos->HashTable->overWrite;
	}

	if (score > ISMATE) score += pos->ply;
	else if (score < -ISMATE) score -= pos->ply;

	pos->HashTable->pTable[index].move = move;
	pos->HashTable->pTable[index].posKey = pos->posKey;
	pos->HashTable->pTable[index].flags = flags;
	pos->HashTable->pTable[index].score = score;
	pos->HashTable->pTable[index].depth = depth;

}


// Return move from posKey
int ProbePvMove(const S_BOARD* pos) {

	int index = pos->posKey % pos->HashTable->numEntries;
	// Valid index
	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
	// Check for matching key

	if (pos->HashTable->pTable[index].posKey == pos->posKey) {
		return pos->HashTable->pTable[index].move;
	}

	return NOMOVE;
}


int ProbeHashEntry(S_BOARD* pos, int* move, int* score, int alpha, int beta, int depth) {

	int index = pos->posKey % pos->HashTable->numEntries;

	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
	ASSERT(depth >= 1 && depth < MAXDEPTH);
	ASSERT(alpha < beta);
	ASSERT(alpha >= -
		
		
		
		
		&& alpha <= INF_BOUND);
	ASSERT(beta >= -INF_BOUND && beta <= INF_BOUND);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

	if (pos->HashTable->pTable[index].posKey == pos->posKey) {
		*move = pos->HashTable->pTable[index].move;
		if (pos->HashTable->pTable[index].depth >= depth) {
			++pos->HashTable->hit;

			ASSERT(pos->HashTable->pTable[index].depth >= 1 && pos->HashTable->pTable[index].depth < MAXDEPTH);
			ASSERT(pos->HashTable->pTable[index].flags >= HFALPHA && pos->HashTable->pTable[index].flags <= HFEXACT);

			*score = pos->HashTable->pTable[index].score;
			if (*score > ISMATE) *score -= pos->ply;
			else if (*score < -ISMATE) *score += pos->ply;

			switch (pos->HashTable->pTable[index].flags) {

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


