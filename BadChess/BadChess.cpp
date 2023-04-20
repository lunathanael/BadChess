// BadChess.cpp : Defines the entry point for the application.
//
#include "stdio.h"
#include "stdlib.h"
#include "defs.h"
#include <cstring> 
#include <iostream>

#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"
#define PERFT "r3k2/p1ppqpb1/bn2pnp11/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define FINE_70 "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -"
#define WAC_ "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -"
#define LCT_1 "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -"

int main(int argc, char *argv[])
{
	AllInit();


	// Start board
	S_BOARD pos[1];
	S_SEARCHINFO info[1];

	info->quit = FALSE;
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



