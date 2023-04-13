// board.cpp

#include "stdio.h"
#include "defs.h"
#include "string.h"

// TEMP:
char START_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Function to check board
int CheckBoard(const S_BOARD* pos) {

	// Arrays hold information to mirror board position
	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int t_bigPce[2] = { 0, 0 };
	int t_majPce[2] = { 0, 0 };
	int t_minPce[2] = { 0, 0 };
	int t_material[2] = { 0, 0 };

	int sq64, t_piece, t_pce_num, sq120, color, pcount;

	U64 t_pawns[3] = { 0Ull, 0ULL, 0ULL };

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH] = pos->pawns[BOTH];

	// Check piece lists
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		for (t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
			sq120 = pos->pList[t_piece][t_pce_num];
			ASSERT(pos->pieces[sq120] == t_piece);
		}
	}

	// Check Piece count and other counters
	for (sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQ120(sq64);
		t_piece = pos->pieces[sq120];
		++t_pceNum[t_piece];
		color = PieceCol[t_piece];
		if (PieceBig[t_piece] == TRUE) ++t_bigPce[color];
		if (PieceMin[t_piece] == TRUE) ++t_minPce[color];
		if (PieceMaj[t_piece] == TRUE) ++t_majPce[color];

		t_material[color] += PieceVal[t_piece];
	}

	// Check Piece number array
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pceNum[t_piece] == pos->pceNum[t_piece]);
	}

	// Check bitboard counts
	pcount = CNT(t_pawns[WHITE]);
	ASSERT(pcount == pos->pceNum[wP]);
	pcount = CNT(t_pawns[BLACK]);
	ASSERT(pcount == pos->pceNum[bP]);
	pcount = CNT(t_pawns[BOTH]);
	ASSERT(pcount == pos->pceNum[bP] + pos->pceNum[wP]);

	// Check bitboards squares
	while (t_pawns[WHITE]) {
		sq64 = POP(&t_pawns[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP);
	}
	while (t_pawns[BLACK]) {
		sq64 = POP(&t_pawns[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP);
	}
	while (t_pawns[BOTH]) {
		sq64 = POP(&t_pawns[BOTH]);
		ASSERT( (pos->pieces[SQ120(sq64)] == wP)  || (pos->pieces[SQ120(sq64)] == bP));
	}

	// Check material
	ASSERT(t_material[WHITE] == pos->material[WHITE] && t_material[BLACK] == pos->material[BLACK]);
	ASSERT(t_minPce[WHITE] == pos->minPce[WHITE] && t_minPce[BLACK] == pos->minPce[BLACK]);
	ASSERT(t_majPce[WHITE] == pos->majPce[WHITE] && t_majPce[BLACK] == pos->majPce[BLACK]);
	ASSERT(t_bigPce[WHITE] == pos->bigPce[WHITE] && t_bigPce[BLACK] == pos->bigPce[BLACK]);

	// Check side
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(GeneratePosKey(pos) == pos->posKey);

	// Check en Passant
	ASSERT(pos->enPas == NO_SQ || (RanksBrd[pos->enPas] == RANK_6 && pos->side == WHITE)
		|| (RanksBrd[pos->enPas] == RANK_3 && pos->side == BLACK));

	// Checking king square
	ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
	ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

	return TRUE;
}

// Update the material list
void UpdateListsMaterial(S_BOARD* pos) {

	int piece, sq, index, color;

	// Loop through every square on the board
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		sq = index;
		piece = pos->pieces[index];
		if (piece != OFFBOARD && piece != EMPTY) {
			color = PieceCol[piece];
			if (PieceBig[piece] == TRUE) ++pos->bigPce[color];
			if (PieceMin[piece] == TRUE) ++pos->minPce[color];
			if (PieceMaj[piece] == TRUE) ++pos->majPce[color];

			// Set Material
			pos->material[color] += PieceVal[piece];

			pos->pList[piece][pos->pceNum[piece]] = sq;
			++pos->pceNum[piece];

			// Update king positions
			if (piece == wK) pos->KingSq[WHITE] = sq;
			if (piece == bK) pos->KingSq[BLACK] = sq;

			// Set bit for Pawn
			if (piece == wP) {
				SETBIT(pos->pawns[WHITE], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}
			else if (piece == bP) {
				SETBIT(pos->pawns[BLACK], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}
		}
	}
}

// Parse FEN string
int ParseFen(char *fen, S_BOARD *pos) {

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
				file = FILE_A;
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

	// Update material
	UpdateListsMaterial(pos);

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
	for (index = 0; index < 2; ++index) {
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
		pos->material[index] = 0;
	}

	// Reset Pawns
	for (index = 0; index < 3; ++index) {
		pos->pawns[index] = 0ULL; // U64 Type
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

// Print the board
void PrintBoard(const S_BOARD* pos) {

	// Variable definitions
	int sq, file, rank, piece;

	printf("\nGame Board:\n\n");

	for (rank = RANK_8; rank >= RANK_1; --rank) {
		printf("%d ", rank + 1); // Print Rank #
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file, rank);
			piece = pos->pieces[sq];
			// Print Piece using index and array
			printf("%3c", PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	// Print Files
	for (file = FILE_A; file <= FILE_H; ++file) {
		printf("%3c", 'a' + file);
	}
	printf("\n");

	// Print Variables
	printf("\n");
	printf("Side:%c\n", SideChar[pos->side]);
	printf("EnPas:%d\n", pos->enPas);
	printf("Castle:%c%c%c%c\n",
		pos->castlePerm & WKCA ? 'K' : '-',
		pos->castlePerm & WQCA ? 'Q' : '-',
		pos->castlePerm & BKCA ? 'k' : '-',
		pos->castlePerm & BQCA ? 'q' : '-'
	);

	// Print posKey
	printf("PosKey:%llX\n", pos->posKey);
}