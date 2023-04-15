// pvtable.cpp


#include "stdio.h"
#include "defs.h"

// Set pvTable size, 2mb size
const int PvSize = 0x100000 * 2;




// Fill PvLine array
int GetPvLine(const int depth, S_BOARD* pos) {

	ASSERT(depth < MAXDEPTH);

	int move = ProbePvTable(pos); // Prove pvTable for move
	int count = 0;

	while (move != NOMOVE && count < depth) {

		// Valid depth
		ASSERT(count < MAXDEPTH);

		if (MoveExists(pos, move)) {
			MakeMove(pos, move);
			pos->pvArray[count++] = move;
		}
		else {
			// Illegal move or count reached
			break;
		}
		move = ProbePvTable(pos);
	}

	// Undo Moves
	while (pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
}


// Function to clear the pvTable
static void ClearPvTable(S_PVTABLE* table) {

	S_PVENTRY* pvEntry;

	for (pvEntry = table->pTable; pvEntry < table->pTable + table->numEntries; ++pvEntry) {
		pvEntry->posKey = 0ULL;
		pvEntry->move = NOMOVE;
	}
}


// Function to initialize pvTable
void InitPvTable(S_PVTABLE* table) {
	
	table->numEntries = PvSize / sizeof(S_PVENTRY); // Entries of type PVENTRY can fit in 2mb
	table->numEntries -= 2; // Prevent out of index
	free(table->pTable);
	// Dynamically declare memory for array
	table->pTable = (S_PVENTRY *) malloc(table->numEntries * sizeof(S_PVENTRY));
	ClearPvTable(table); // Reset all keys and moves
	printf("PvTable init complete with %d entries\n", table->numEntries);

}


// Store move into pvTable
void StorePvMove(const S_BOARD* pos, const int move) {

	int index = pos->posKey % pos->PvTable->numEntries;

	// Valid index
	ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);

	// Store move and key
	pos->PvTable->pTable[index].move = move;
	pos->PvTable->pTable[index].posKey = pos->posKey;

}


// Return move from posKey
int ProbePvTable(const S_BOARD* pos) {

	int index = pos->posKey % pos->PvTable->numEntries;
	// Valid index
	ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
	// Check for matching key
	if (pos->PvTable->pTable[index].posKey = pos->posKey) {
		return pos->PvTable->pTable[index].move;
	}

	return NOMOVE;
}


