#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"

#define DEBUG // Comment out to run at full speed

// DEBUG function
#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed", #n); \
printf("On %s ", __DATE__); \
printf("At %s ", __TIME__); \
printf("In File %s ", __FILE__); \
printf("At Line %d\n", __LINE__); \
exit(1);}
#endif


typedef unsigned long long U64; // Unsigned 64 bit number

#define NAME "BadChess"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048 // Maximum game half moves to store moves

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK }; // Enumerating pieces
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE }; // Enumerating Files
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE }; // Enumerating Ranks
enum { WHITE, BLACK, BOTH }; // Enumerating Color
enum {
	A1 = 21, B1, C1, D1, E1, F1, G1, H1,
	A2 = 31, B2, C2, D2, E2, F2, G2, H2,
	A3 = 41, B3, C3, D3, E3, F3, G3, H3,
	A4 = 51, B4, C4, D4, E4, F4, G4, H4,
	A5 = 61, B5, C5, D5, E5, F5, G5, H5,
	A6 = 71, B6, C6, D6, E6, F6, G6, H6,
	A7 = 81, B7, C7, D7, E7, F7, G7, H7,
	A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
}; // Enumberating board structure

enum { TRUE, FALSE }; // True False enum

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 }; // White and Black castling rights in bits, I.e: 1 1 1 1

// Stucture for Move Undo
typedef struct {

	int move; // Moves to undo
	int castlePerm; // Castling rights
	int enPas; // Enpassant Square
	int fiftyMove; // Fifty Move rule
	U64 posKey; // position Key

} S_UNDO;

// Structure for Board
typedef struct {
	int pieces[BRD_SQ_NUM];
	U64 pawns[3]; // Pawns, includes white, black, and both

	int KingSq[2]; // Squares that kings are on

	int side; // Side to Move
	int enPas; // Square of En Passant
	int fiftyMove; // Fifty Move rule

	int ply; // Halfmoves in search
	int hisPly; // Halfmoves in total game

	int castlePerm; // Castling rights

	U64 posKey; // Position/Hash Key to represent position on board

	int pceNum[13]; // Number of pieces on the board, indexed by piece type
	int big[3]; // Number of big pieces that are not pawns
	int majPce[3]; // Number of major pieces
	int minPce[3]; // Number of minor pieces

	S_UNDO history[MAXGAMEMOVES]; // Store information to board history

	// Piece list
	int pList[13][10]; // 13 Piece types, 10 possible Pieces at a time: pList[wN][0] = E1; ....

} S_BOARD;

/* MACROS */

#define FR2SQ(f, r) ( (21 + (f) ) + ( (r) * 10 ) ) // Convert file/rank to square number
#define SQ64(sq120) (Sq120ToSq64[sq120]) // Take 120 based board to return index on 64 based board
#define SQ120(sq64) (Sq64ToSq120[sq64]) // Take 64 based board to return index on 120 based board
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])


/* GLOBALS */

extern int Sq120ToSq64[BRD_SQ_NUM]; // Convert 120 board to 64 board
extern int Sq64ToSq120[64]; // Conver 64 board to 120 board
extern U64 SetMask[64]; // Array to set bit in bitboard
extern U64 ClearMask[64]; // Array to clear bit in bitboard
extern U64 PieceKeys[13][120]; // Pieces indexed by squares
extern U64 SideKey; // Side to move
extern U64 CastleKeys[16]; // Castle Rights, I.e: 1 0 0 1


/* FUNCTIONS */

// init.cpp
extern void AllInit();

// bitboard.cpp
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);

// hashkeys.cpp
extern U64 GeneratePosKey(const S_BOARD* pos);

#endif