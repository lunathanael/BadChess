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
	pos->HashTable->pTable = NULL;
	InitHashTable(pos->HashTable, 512);

	printf("Welcome to BadChess! Type 'cons' for console mode...\n");

	std::string input;
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

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
		else if (input == "xboard") {
			XBoard_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		else if (input == "cons") {
			Console_Loop(pos, info);
			if (info->quit == TRUE) break;
			continue;
		}
		if (input == "quit") {
			break;
		}
	}

	// Clean allocated memory
	free(pos->HashTable->pTable);
	CleanPolyBook();
	return 0;
};



