#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <string>


#if (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#include <nmmintrin.h> // Used for countBits
#if defined(_WIN64)
#include <intrin.h> // Used for _BitScanForward64 and _BitScanReverse64
#endif
#endif

namespace BBS {
	const Bitboard EMPTY = std::stoull("0000000000000000000000000000000000000000000000000000000000000000", nullptr, 2);
	const Bitboard UNIVERSE = std::stoull("1111111111111111111111111111111111111111111111111111111111111111", nullptr, 2);

	constexpr Bitboard RankMasks8[8] =/*from rank1 to rank8*/
	{
		255ULL, 
		65280ULL,
		16711680ULL,
		4278190080ULL,
		1095216660480ULL,
		280375465082880ULL,
		71776119061217280ULL,
		18374686479671623680ULL
	};
	
	constexpr Bitboard FileMasks8[8] =/*from fileA to FileH*/
	{
		0x101010101010101ULL,
		0x202020202020202ULL,
		0x404040404040404ULL,
		0x808080808080808ULL,
		0x1010101010101010ULL,
		0x2020202020202020ULL,
		0x4040404040404040ULL,
		0x8080808080808080ULL
	};

	// Indexed by 7 + rank - file
// The diagonals start at the h1-h1 and end in the a8-a8
	constexpr uint64_t diagonalMasks[15] = { 
		128ULL, 
		32832ULL,
		8405024ULL,
		2151686160ULL,
		550831656968ULL,
		141012904183812ULL,
		36099303471055874ULL,
		9241421688590303745ULL,
		4620710844295151872ULL, 
		2310355422147575808ULL, 
		1155177711073755136ULL, 
		577588855528488960ULL,
		288794425616760832ULL, 
		144396663052566528ULL, 
		72057594037927936ULL
	};

	// Indexed by: rank + file
	// Anti-diagonals go from a1-a1 to h8-h8
	constexpr uint64_t antidiagonalMasks[15] = { 
		1ULL,
		258ULL,
		66052ULL,
		16909320ULL,
		4328785936ULL,
		1108169199648ULL,
		283691315109952ULL,
		72624976668147840ULL,
		145249953336295424ULL,
		290499906672525312ULL,
		580999813328273408ULL,
		1161999622361579520ULL,
		2323998145211531264ULL,
		4647714815446351872ULL,
		9223372036854775808ULL
	};


	// knight_attacks[fromSq]
	extern Bitboard knight_attacks[64];

	// king_attacks[fromSq]
	extern Bitboard king_attacks[64];


	namespace Zobrist {
		// Indexed by piece_keys[color][type][sq]
		extern Bitboard piece_keys[2][6][64];

		// Indexed by empty_keys[sq]
		extern Bitboard	empty_keys[64];

		// This is just xor'ed in if the side to move is white.
		extern Bitboard side_key;

		// Indexed by castling_keys[pos->castleRights]
		extern Bitboard castling_keys[16];
		
		void init_zobrist();
	}


	namespace EvalBitMasks {
		extern Bitboard passed_pawn_masks[2][64];
		extern Bitboard isolated_bitmasks[8];

		extern Bitboard outpost_masks[2][64];

		void initBitMasks();
	}


	void init_knightAttacks();
	void init_kingAttacks();

	void INIT();
}


namespace Magics {
	// The magic numbers for each square.
	extern const Bitboard rook_magics[64];
	extern const Bitboard bishop_magics[64];

	// Relevant amount of occupancy bits for each square.
	extern const int rook_relevant_bits[64];
	extern const int bishop_relevant_bits[64];

	// All attack masks of rooks and bishops excluding edges.
	extern Bitboard magic_rook_masks[64];
	extern Bitboard magic_bishop_masks[64];


	// These are the tables holding all attacks to different occupancies on each square.
	extern Bitboard _bishopAttacks[64][512];
	extern Bitboard _rookAttacks[64][4096];

	// These functios are only used to initialize _bishopAttacks and _rookAttacks
	template<piece PCE> Bitboard _getSlowAttack(int sq, Bitboard occupied);
	void _initialize_slider_tables(bool is_rook);
	Bitboard set_occupancy(int index, int bit_cnt, Bitboard mask);

	// To initialize the bishopMagics, rookMagics and all attack tables.
	void INIT();


	// This is the functions returning the attack bitboards for sliders on different squares with different occupancies.
	template<piece PCE>
	Bitboard attacks_bb(int sq, Bitboard occ);
}



// Shifts the bitboard one square in some direction.
template <DIRECTION d>
inline Bitboard shift(Bitboard bb) {
	return d == NORTH ? bb << 8
		: d == SOUTH ? bb >> 8
		: d == EAST ? (bb & ~BBS::FileMasks8[FILE_H]) << 1
		: d == WEST ? (bb & ~BBS::FileMasks8[FILE_A]) >> 1
		: d == NORTHEAST ? (bb & ~(BBS::FileMasks8[FILE_H] | BBS::RankMasks8[RANK_8])) << 9
		: d == NORTHWEST ? (bb & ~(BBS::FileMasks8[FILE_A] | BBS::RankMasks8[RANK_8])) << 7
		: d == SOUTHEAST ? (bb & ~(BBS::FileMasks8[FILE_H] | BBS::RankMasks8[RANK_1])) >> 7
		: d == SOUTHWEST ? (bb & ~(BBS::FileMasks8[FILE_A] | BBS::RankMasks8[RANK_1])) >> 9
		: 0;
}


// Used for debugging bitboards in main.cpp
inline void printBitboard(Bitboard bb) {
	for (int rank = 7; rank >= 0; rank--) {

		for (int file = 0; file < 8; file++) {
			if (((bb >> (8 * rank + file)) & 1) == 1) {
				std::cout << "X";
			}
			else {
				std::cout << "-";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
}



// Used in evaluation.cpp
constexpr uint64_t m1 = 0x5555555555555555;
constexpr uint64_t m2 = 0x3333333333333333;
constexpr uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
constexpr uint64_t h01 = 0x0101010101010101;
inline int countBits(uint64_t x) { // Count amount of turned on bits in a uint64_t number
#if ((defined(__INTEL_COMPILER) || defined(_MSC_VER)) && defined(_WIN64)) && defined(USE_POPCNT)
	return (int)_mm_popcnt_u64(x);
#else
	x -= (x >> 1) & m1;
	x = (x & m2) + ((x >> 2) & m2);
	x = (x + (x >> 4)) & m4;
	return (x * h01) >> 56;
#endif
}




constexpr int index64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

// Finds the most significant 1-bit
inline int bitScanReverse(Bitboard bb) { // Find the MS1B
	assert(bb != 0);

#if defined(__GNUC__) // GCC
	return int(63 ^ __builtin_clzll(bb));
#elif defined(_MSC_VER)

#if defined(_WIN64) // Windows 64-bit
	unsigned long idx;
	_BitScanReverse64(&idx, bb);
	return (int)idx;
#else // Windows 32-bit
	unsigned long idx;

	if (bb >> 32) {
		_BitScanReverse(&idx, int32_t(bb >> 32));
		return int(idx + 32);
	}
	else {
		_BitScanReverse(&idx, int32_t(bb));
		return int(idx);
	}
#endif

#else // Other

	const Bitboard debruijn64 = 0x03f79d71b4cb0a89;
	bb |= bb >> 1;
	bb |= bb >> 2;
	bb |= bb >> 4;
	bb |= bb >> 8;
	bb |= bb >> 16;
	bb |= bb >> 32;
	return index64[(bb * debruijn64) >> 58];
#endif
}


// Finds the least significant 1-bit
inline int bitScanForward(Bitboard bb) { // Find the LS1B
	assert(bb != 0);

#if defined(__GNUC__) // GCC intrinsic.
	return int(__builtin_ctzll(bb));
#elif defined(_MSC_VER)

#if defined(_WIN64) // Windows 64-bit
	unsigned long idx;
	_BitScanForward64(&idx, bb);
	return (int)idx;
#else  // Windows 32-bit
	unsigned long idx;

	if (bb & 0xffffffff) {
		_BitScanForward(&idx, int32_t(bb));
		return int(idx);
}
	else {
		_BitScanForward(&idx, int32_t(bb >> 32));
		return int(idx + 32);
	}
#endif

#else // Other OS/compiler
	const Bitboard debruijn64 = 0x03f79d71b4cb0a89;
	return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
#endif
}


constexpr int BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

// Finds the least significant 1-bit, sets it to a zero and returns the place.
inline int PopBit(Bitboard* bb) {
	int i = bitScanForward(*bb);
	*bb ^= (uint64_t(1) << i);
	
	return i;
}





#endif