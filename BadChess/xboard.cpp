// xboard.cpp

#include "stdio.h"
#include "defs.h"
#include "string.h"
#include <iostream>


// Checks if the move made is a rep
int ThreeFoldRep(const S_BOARD* pos) {

	int i = 0; int r = 0;
	for (i = 0; i < pos->hisPly; ++i) {
		if (pos->history[i].posKey == pos->posKey) {
			++r;
		}
	}
	return r;
}


// Checks if it is a material draw
int DrawMaterial(const S_BOARD* pos) {

	if (pos->pceNum[wP] || pos->pceNum[bP]) return FALSE; // Pawns
	if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return FALSE; // Queens and rooks
	if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) return FALSE; // Bishops
	if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) return FALSE; // Knights
	if (pos->pceNum[wN] && pos->pceNum[wB]) return FALSE; // Bishop and knight
	if (pos->pceNum[bN] && pos->pceNum[bB]) return FALSE; // Bishop and knight

	return TRUE; // Is material draw
}


void PrintOptions() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
	printf("feature done=1\n");
}


// Check for draws
int checkresult(S_BOARD* pos) {

	// Fifty move draw
	if (pos->fiftyMove > 100) {
		printf("1/2-1/2 {fifty move rule (claimed by BadChess)}\n"); return TRUE;
	}

	// Three fold draw
	if (ThreeFoldRep(pos) >= 2) {
		printf("1/2-1/2 {3-fold repetition (claimed by BadChess)}\n"); return TRUE;
	}

	// Material draw
	if (ThreeFoldRep(pos) >= 2) {
		printf("1/2-1/2 {insufficient material (claimed by BadChess)}\n"); return TRUE;
	}

	// Check for stalemate or checkmate
	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	int MoveNum = 0;
	int found = 0;
	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}
		++found;
		TakeMove(pos);
		break;
	}
	
	// Legal move found
	if (found != 0) return FALSE;

	int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);
	
	// Checkmate
	if (InCheck == TRUE) {
		if (pos->side == WHITE) {
			printf("0-1 {black mates (claimed by BadChess)}\n"); return TRUE;
		}
		else {
			printf("0-1 {white mates (claimed by BadChess)}\n"); return TRUE;
		}
	}
	// Stalemate
	else {
		printf("0-1 {stalemate (claimed by BadChess)}\n");
	}

	return FALSE;
}


// Xboard function
void XBoard_Loop(S_BOARD* pos, S_SEARCHINFO* info) {

	info->GAME_MODE = XBOARDMODE;
	info->POST_THINKING = TRUE;
	PrintOptions(); // HACK

	// Definitions
	bool parsed_position = false;
	int depth = -1, movestogo[2] = { 30, 30 }, movetime = -1;
	int time = -1, inc = 0;
	int engineSide = BOTH;
	int timeLeft;
	int sec;
	int mps;
	int move = NOMOVE;
	int i, score;
	//int MB;

	engineSide = BLACK;
	ParseFen(START_FEN, pos);
	depth = -1;
	time = -1;


	// Main loop
	while (TRUE) {

		std::string input;

		if (pos->side == engineSide && checkresult(pos) == FALSE) {
			info->starttime = GetTimeMs();
			info->depth = depth;

			if (time != -1) {
				info->timeset = TRUE;
				time /= movestogo[pos->side];
				time -= 50;
				info->stoptime = info->starttime + time + inc;
			}

			if (depth == -1 || depth > MAXDEPTH) {
				info->depth = MAXDEPTH;
			}

			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time, info->starttime, info->stoptime, info->depth, info->timeset, movestogo[pos->side], mps);
			SearchPosition(pos, info);

			if (mps != 0) {
				movestogo[pos->side ^ 1]--;
				if (movestogo[pos->side ^ 1] < 1) {
					movestogo[pos->side ^ 1] = mps;
				}
			}
		}

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

		//Split the string into tokens to make it easier to work with
		//std::vector<std::string> tokens = split_command(input);

		// Quit command
		if (input == "quit") {
			info->quit = TRUE;
			break;
		}

		// Force command
		else if (input == "force") {
			engineSide = BOTH;
			continue;
		}

		// Set all features
		else if (input == "protover") {
			PrintOptions();

		}

		// Set depth command
		else if (input == "sd") {
			depth = std::stoi(input.substr(input.find("sd") + 3, std::string::npos));
		}

		// Set move time
		else if (input == "st") {
			movetime = std::stoi(input.substr(input.find("sd") + 3, std::string::npos));
		}

		// Synchronize
		else if (input == "ping") {
			printf("pong %s\n", input.substr(input.find("ping") + 5, std::string::npos));
		}

		// New board
		else if (input == "new") {
			engineSide = BLACK;
			ParseFen(START_FEN, pos);
			depth = -1;
			continue;
		}

		// Set FEN
		else if (input == "setboard") {
			engineSide = BOTH;
			ParseFen(input.substr(input.find("setboard") + 9, std::string::npos), pos);
			continue;
		}

		// Go command
		else if (input == "go") {
			engineSide = pos->side;
			continue;
		}

		// Usermove command
		else if (input == "usermove") {
			--movestogo[pos->side];
			move = ParseMove(input.substr(input.find("usermove") + 9, std::string::npos), pos);
			if (move == NOMOVE) continue;
			MakeMove(pos, move);
			pos->ply = 0;
		}
	}
}





void Console_Loop(S_BOARD* pos, S_SEARCHINFO* info) {

	printf("Welcome to BadChess In Console Mode!\n");
	printf("Type help for commands\n\n");

	info->GAME_MODE = CONSOLEMODE;
	info->POST_THINKING = TRUE;
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	int depth = MAXDEPTH, movetime = 3000;
	int engineSide = BOTH;
	int move = NOMOVE;
	char inBuf[80], command[80];

	engineSide = BLACK;
	ParseFen(START_FEN, pos);

	while (TRUE) {

		fflush(stdout);

		if (pos->side == engineSide && checkresult(pos) == FALSE) {
			info->starttime = GetTimeMs();
			info->depth = depth;

			if (movetime != 0) {
				info->timeset = TRUE;
				info->stoptime = info->starttime + movetime;
			}

			SearchPosition(pos, info);
		}

		printf("\nBadChess > ");

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);
		if (!fgets(inBuf, 80, stdin))
			continue;

		sscanf(inBuf, "%s", command);

		if (!strcmp(command, "help")) {
			printf("Commands:\n");
			printf("quit - quit game\n");
			printf("force - computer will not think\n");
			printf("print - show board\n");
			printf("post - show thinking\n");
			printf("nopost - do not show thinking\n");
			printf("new - start new game\n");
			printf("go - set computer thinking\n");
			printf("depth x - set depth to x\n");
			printf("time x - set thinking time to x seconds (depth still applies if set)\n");
			printf("view - show current depth and movetime settings\n");
			printf("setboard x - set position to fen x\n");
			printf("** note ** - to reset time and depth, set to 0\n");
			printf("enter moves using b7b8q notation\n\n\n");
			continue;
		}

		//if (!strcmp(command, "mirror")) {
		//	engineSide = BOTH;
		//	MirrorEvalTest(pos);
		//	continue;
		//}

		if (!strcmp(command, "eval")) {
			PrintBoard(pos);
			printf("Eval:%d", EvalPosition(pos));
			//MirrorBoard(pos);
			PrintBoard(pos);
			printf("Eval:%d", EvalPosition(pos));
			continue;
		}

		if (!strcmp(command, "setboard")) {
			engineSide = BOTH;
			ParseFen(inBuf + 9, pos);
			continue;
		}

		if (!strcmp(command, "quit")) {
			info->quit = TRUE;
			break;
		}

		if (!strcmp(command, "post")) {
			info->POST_THINKING = TRUE;
			continue;
		}

		if (!strcmp(command, "print")) {
			PrintBoard(pos);
			continue;
		}

		if (!strcmp(command, "nopost")) {
			info->POST_THINKING = FALSE;
			continue;
		}

		if (!strcmp(command, "force")) {
			engineSide = BOTH;
			continue;
		}	
		
		if (!strcmp(command, "polykey")) {
			PrintBoard(pos);
			GetBookMove(pos);
			continue;
		}

		if (!strcmp(command, "view")) {
			if (depth == MAXDEPTH) printf("depth not set ");
			else printf("depth %d", depth);

			if (movetime != 0) printf(" movetime %ds\n", movetime / 1000);
			else printf(" movetime not set\n");

			continue;
		}

		if (!strcmp(command, "depth")) {
			sscanf(inBuf, "depth %d", &depth);
			if (depth == 0) depth = MAXDEPTH;
			continue;
		}

		if (!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &movetime);
			movetime *= 1000;
			continue;
		}

		if (!strcmp(command, "new")) {
			ClearHashTable(pos->HashTable);
			engineSide = BLACK;
			ParseFen(START_FEN, pos);
			continue;
		}

		if (!strcmp(command, "go")) {
			engineSide = pos->side;
			continue;
		}

		move = ParseMove(inBuf, pos);
		if (move == NOMOVE) {
			printf("Command unknown:%s\n", inBuf);
			continue;
		}
		MakeMove(pos, move);
		pos->ply = 0;
	}
}



