// pvtable.cpp


#include "stdio.h"
#include "defs.h"

// Set pvTable size, 2mb size
const int PvSize = 0x100000 * 2;



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
	free(table->pTable); // Free the memory
	printf("HI");
	// Dynamically declare memory for array
	table->pTable = (S_PVENTRY *) malloc(table->numEntries * sizeof(S_PVENTRY));
	ClearPvTable(table); // Reset all keys and moves
	printf("PvTable init complete with %d entries\n", table->numEntries);

}




