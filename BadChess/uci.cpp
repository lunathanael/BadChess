// uci.cpp

#include "stdio.h"
#include "defs.h"
#include "string.h"
#include <iostream>




// parse UCI "go" command, returns true if we have to search afterwards and false otherwise
void ParseGo(const std::string& line, S_SEARCHINFO* info, S_BOARD* pos) {

	
	int depth = -1, movetime = -1; int movestogo = 30;
	int time = -1; int inc = 0;
	info->timeset = FALSE;

	std::vector<std::string> tokens = split_command(line);

	//loop over all the tokens and parse the commands
	for (size_t i = 1; i < tokens.size(); i++) {

		if (tokens.at(1) == "infinite") {
			;
		}

		//if (tokens.at(1) == "perft") {
		//	int perft_depth = std::stoi(tokens[2]);
		//	PerftTest(perft_depth, pos);
		//	return false;
		//}

		if (tokens.at(i) == "binc" && pos->side == BLACK) {
			inc = std::stoi(tokens[i + 1]);
		}

		if (tokens.at(i) == "winc" && pos->side == WHITE) {
			inc = std::stoi(tokens[i + 1]);
		}

		if (tokens.at(i) == "wtime" && pos->side == WHITE) {
			time = std::stoi(tokens[i + 1]);
			info->timeset = TRUE;
		}
		if (tokens.at(i) == "btime" && pos->side == BLACK) {
			time = std::stoi(tokens[i + 1]);
			info->timeset = TRUE;
		}

		if (tokens.at(i) == "movestogo") {
			movestogo = std::stoi(tokens[i + 1]);
			if (movestogo > 0)
				info->movestogo = movestogo;
		}

		if (tokens.at(i) == "movetime") {
			movetime = std::stoi(tokens[i + 1]);
			time = movetime;
			movestogo = 1;
			info->timeset = TRUE;
		}

		if (tokens.at(i) == "depth") {
			depth = std::stoi(tokens[i + 1]);
		}

		//if (tokens.at(i) == "nodes") {
		//	info->nodeset = true;
		//	info->nodeslimit = std::stoi(tokens[i + 1]);
		//}
	}

	info->starttime = GetTimeMs();
	info->depth = depth;

	//calculate time allocation for the move
	if (info->timeset) {

	}

	int safety_overhead = 50;

	if (depth == -1) {
		info->depth = MAXDEPTH;
		time /= movestogo;
		time -= safety_overhead;
		info->stoptime = info->starttime + time + inc;
	}

	std::cout << "info ";
	std::cout << "time: " << time << " ";
	std::cout << "start: " << info->starttime << " ";
	std::cout << "stop: " << info->stoptime << " ";
	std::cout << "depth: " << info->depth << " ";
	std::cout << "timeset: " << info->timeset << " ";
	
	SearchPosition(pos, info);
}



//convert a move to coordinate notation to internal notation
int ParseMove(const std::string& move_string, S_BOARD* pos) {

	// parse source square
	const int from = FR2SQ(move_string[0] - 'a', move_string[1] - '1');

	// parse target square
	const int to = FR2SQ(move_string[2] - 'a', move_string[3] - '1');

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);
	int MoveNum = 0;
	int Move = 0;
	int PromPce = EMPTY;

	// Search for move in movelist
	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		Move = list->moves[MoveNum].move;
		if (FROMSQ(Move) == from && TOSQ(Move) == to) {
			PromPce = PROMOTED(Move);
			if (PromPce != EMPTY) {
				if (IsRQ(PromPce) && !IsBQ(PromPce) && move_string[4] == 'r') {
					return Move;
				}
				else if (!IsRQ(PromPce) && IsBQ(PromPce) && move_string[4] == 'b') {
					return Move;
				}
				else if (IsRQ(PromPce) && IsBQ(PromPce) && move_string[4] == 'q') {
					return Move;
				}
				else if (IsKN(PromPce) && move_string[4] == 'n') {
					return Move;
				}
				continue;
			}
			return Move;
		}
	}

	return NOMOVE;

	// return illegal move
	return 0;
}


// parse UCI "position" command
void ParsePosition(const std::string& command, S_BOARD* pos) {

	// parse UCI "startpos" command
	if (command.find("startpos") != std::string::npos) {
		// init chess board with start position
		ParseFen(START_FEN, pos);
	}

	// parse UCI "fen" command
	else {

		// if a "fen" command is available within command string
		if (command.find("fen") != std::string::npos) {
			// init chess board with position from FEN string
			ParseFen(command.substr(command.find("fen") + 4, std::string::npos), pos);
		}
		else {
			// init chess board with start position
			ParseFen(START_FEN, pos);
		}

	}

	// if there are moves to be played in the fen play them
	if (command.find("moves") != std::string::npos) {
		int string_start = command.find("moves") + 6;
		std::string moves_substr = command.substr(string_start, std::string::npos);
		parse_moves(moves_substr, pos);
	}
	PrintBoard(pos);
}


// Uci function
void Uci_Loop() {


	// Definitions
	bool parsed_position = false;

	// Start uci
	printf("id name %s\n", NAME);
	printf("id author Nate\n");
	printf("uciok\n");

	// Start board
	S_BOARD pos[1];
	S_SEARCHINFO info[1];
	InitPvTable(pos->PvTable);

	// Main loop
	while (TRUE) {

		std::string input;


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
		std::vector<std::string> tokens = split_command(input);

		// parse UCI "position" command
		if (tokens[0] == "position") {
			// call parse position function
			ParsePosition(input, pos);
			parsed_position = true;
		}

		// parse UCI "go" command
		else if (tokens[0] == "go") {
			 

			if (!parsed_position) // call parse position function
			{
				ParsePosition("position startpos", pos);
			}
			// call parse go function
			ParseGo(input, info, pos);

		}

		// parse UCI "isready" command
		else if (input == "isready") {
			std::cout << "readyok\n";

			continue;
		}

		// parse UCI "ucinewgame" command
		else if (input == "ucinewgame") {
			ParsePosition("position startpos\n", pos);
		}

		else if (input == "quit") {
			info->quit = TRUE;
			break;
		}
		else if (input == "uci") {
			printf("id name %s\n", NAME);
			printf("id author Nate\n");
			printf("uciok\n");
		}

		if (info->quit) break;
	}

	free(pos->PvTable->pTable);
}

