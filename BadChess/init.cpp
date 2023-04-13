// init.cpp

#include "defs.h"
#include "stdLib.h"

// Fill 64 bits with random numbers, I.e: 0000 000000000000000 000000000000000 000000000000000 000000000000000
#define RAND_64 (	(U64)rand() + \
					(U64)rand() << 15 + \
					(U64)rand() << 30 + \
					(U64)rand() << 45 + \
					((U64)rand() & 0xf) << 60	)

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

// Keys for position hashkeys
U64 PieceKeys[13][120]; // Pieces indexed by squares
U64 SideKey; // Side to move
U64 CastleKeys[16]; // Castle Rights, I.e: 1 0 0 1

void InitHashKeys() {
	
	// Indices
	int index = 0;
	int index2 = 0;
	// Fill PieceKeys with random numbers
	for (index = 0; index < 13; ++index) {
		for (index2 = 0; index2 < 120; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	// Fill CastleKeys with random numbers
	SideKey = RAND_64;
	for (index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}
}

// Initialize bitmasks to set and clear bits
void InitBitMasks() {
	int index = 0;

	// Zero the values inside the array
	for (index = 0; index < 64; ++index) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	// Set the values for the Masks
	for (index = 0; index < 64; ++index) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index]; // Set to bitwise complement of SetMask
	}
}

// Init function for conversion from 120 to 64
void InitSq120To64() {

	// Declare Variables
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;
	// Iterating through all the squares in the array
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65; // Setting the values all to 65, failsafe value
	}
	// Iterating through all the squares in the array
	for (index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120; // Setting the values all to 120, failsafe value
	}
	// Iterate over rank and file
	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file, rank); // Get Square
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			++sq64;
		}
	}
}

void AllInit() {
	InitSq120To64();
	InitBitMasks();
	InitHashKeys();
}