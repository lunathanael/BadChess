// board.cpp

#include "stdio.h"
#include "defs.h"


// Parse FEN string
int Parse_Fen(char *fen, S_BOARD *pos) {

	// Check that there is somethign to point at
	ASSERT(fen != NULL);
	ASSERT(pos != NULL);

	// Declare variables
	int rank = RANK_8; // Start from 8th Rank
	int file = FILE_A; // Start from A file
	int piece = 0;
	int count = 0;
	int i = 0;
	int sq64 = 0;
	int sq120 = 0;

	// Reset the board
	ResetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
		count = 1;
		switch (*fen) {
			// If is piece
			case 'p': piece = bP; break;
			case 'r': piece = bR; break;
			case 'n': piece = bN; break;
			case 'b': piece = bB; break;
			case 'k': piece = bK; break;
			case 'q': piece = bQ; break;
			case 'P': piece = wP; break;
			case 'R': piece = wR; break;
			case 'N': piece = wN; break;
			case 'B': piece = wB; break;
			case 'K': piece = wK; break;
			case 'Q': piece = wQ; break;

			// Is empty Square
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				piece = EMPTY;
				count = *fen - '0';
				break;
			
			case '/':
			case ' ':
				// End of pieces or new rank
				--rank;
				++fen;
				continue;

			default:
				printf("FEN error \n");
				return -1;
		}

		// Insert empty squares
		for (i = 0; i < count; ++i) {
			sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);
			if (piece != EMPTY) {
				pos->pieces[sq120] = piece;
			}
			++file;
		}
		++fen;
	}

	// Insert turn
	ASSERT(*fen == 'w' || *fen == 'b');
	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	// Set Castling rights
	for (i = 0; i < 4; ++i) {
		// If pointing to a space
		if (*fen == ' ') {
			break;
		}
		// Process the character being looked at
		switch (*fen) {
			case 'K': pos->castlePerm |= WKCA; break;
			case 'Q': pos->castlePerm |= WQCA; break;
			case 'k': pos->castlePerm |= BKCA; break;
			case 'q': pos->castlePerm |= BQCA; break;
			default: break;
		}
		++fen;
	}
	++fen;

	// Set en Passant Square
	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file >= FILE_A && file <= FILE_H);
		ASSERT(rank >= RANK_1 && rank <= RANK_8);

		pos->enPas = FR2SQ(file, rank);
	}

	// Generate posKey
	pos->posKey = GeneratePosKey(pos);

	return 0;
}

// Reset the board
void ResetBoard(S_BOARD *pos) {
	
	int index = 0;

	// Set all squares to "offboard"
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	// Define squares on the board to empty
	for (index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	// Reset arrays in board structure
	for (index = 0; index < 3; ++index) {
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
		pos->pawns[index] = 0Ull; // U64 type
	}

	// Reset piece number by piece type
	for (index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	// Reset kingPosition for white and black
	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	// Reset the side to both
	pos->side = BOTH;
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;

	// Reset the nubmer of halfmoves
	pos->ply = 0;
	pos->hisPly = 0;

	// Reset Castle rights
	pos->castlePerm = 0;

	// Reset Position Key
	pos->posKey = 0ULL;
}