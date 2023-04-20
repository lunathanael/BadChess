// BadChess.cpp : Defines the entry point for the application.
//
#include "stdio.h"
#include "stdlib.h"
#include "defs.h"
#include <cstring> 
#include <iostream>

int main(int argc, char *argv[])
{
	AllInit();


	// Start board
	S_BOARD pos[1];
	S_SEARCHINFO info[1];

	info->quit = FALSE;
	info->threadNum = 1; // Defuault 1 thread


	HashTable->pTable = NULL;
	InitHashTable(HashTable, 512);

	std::string input;
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	/*TempHashTest(PERFT);
	TempHashTest(WAC1);
	exit(0);*/

	printf("Welcome to BadChess! Type 'cons' for console mode...\n");

	int ArgNum = 0;
	for (ArgNum = 0; ArgNum < argc; ++ArgNum) {
		if (strncmp(argv[ArgNum], "NoBook", 6) == 0) {
			EngineOptions->UseBook = FALSE;
		}
	}

	while (TRUE) {

		// get user / GUI input
		if (!std::getline(std::cin, input)) {
			// continue the loop
			break;
		}

		// make sure input is available
		if (!input.length()) {
			// continue the loop
			continue;
		}

		if (input == "\n")
			continue;
		if (input == "uci") {
			Uci_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		if (input == "quit") {
			break;
		}
	}

	// Clean allocated memory
	free(HashTable->pTable);
	CleanPolyBook();
	return 0;
};



