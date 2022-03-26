#ifndef LOKI_PCH_H
#define LOKI_PCH_H
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <array>
#include <mutex>
#include <iostream>
#include <assert.h>

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#include <nmmintrin.h> // Used for count_bits
#if defined(_WIN64)
#include <intrin.h> // Used for scan_forward and scan_reverse.
#endif
#endif

namespace loki {
	// General type definitions.
	using move_t		= uint16_t;
	using bitboard_t	= uint64_t;

	enum SIDE : int64_t {
		WHITE = 0,
		BLACK = 1,
		SIDE_NB = 2
	};

	enum PIECE : int64_t {
		PAWN = 0,
		KNIGHT = 1,
		BISHOP = 2,
		ROOK = 3,
		QUEEN = 4,
		KING = 5,
		PIECE_NB = 6,
		PIECE_NB_TOTAL = 12
	};

	enum SQUARE : int64_t {
		A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
		A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
		A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
		A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
		A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
		A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
		A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
		A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63, SQ_NB = 64, NO_SQ = 65
	};

	enum CASTLING_RIGHTS : uint8_t {
		WKCA = 0,
		WQCA = 1,
		BKCA = 2,
		BQCA = 3
	};

	enum RANK : int64_t {
		RANK_1 = 0,
		RANK_2 = 1,
		RANK_3 = 2,
		RANK_4 = 3,
		RANK_5 = 4,
		RANK_6 = 5,
		RANK_7 = 6,
		RANK_8 = 7,
		RANK_NB = 8,
		NO_RANK = 9
	};
	enum FILE : int64_t {
		FILE_A = 0,
		FILE_B = 1,
		FILE_C = 2,
		FILE_D = 3,
		FILE_E = 4,
		FILE_F = 5,
		FILE_G = 6,
		FILE_H = 7,
		FILE_NB = 8,
		NO_FILE = 9
	};

	enum DIRECTION :size_t {
		NORTH = 0,
		SOUTH = 1,
		EAST = 2,
		WEST = 3,
		NORTHWEST = 4,
		NORTHEAST = 5,
		SOUTHWEST = 6,
		SOUTHEAST = 7
	};

	inline constexpr SQUARE& operator++(SQUARE& orig, int) {
		auto rval = static_cast<int64_t>(orig);
		rval++;
		
		orig = static_cast<SQUARE>(rval);
		return orig;
	}
	inline constexpr SQUARE operator--(SQUARE& orig, int) {
		return static_cast<SQUARE>(static_cast<int64_t>(orig) - 1);
	}

	// Constant declarations.
	constexpr size_t MAX_POSITION_MOVES		= 256; // The maximum amount of pseudo-legal moves in a given position. TODO: Remove this restraint.
	constexpr size_t MAX_GAME_MOVES			= 1024; // Maximum amount of moves in a game.  TODO: Remove this restraint.

	// Bit masks.
	namespace bitmasks {

		constexpr bitboard_t rank_masks[8] =/*from rank1 to rank8*/
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

		constexpr bitboard_t file_masks[8] =/*from fileA to FileH*/
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

	}

	// Inline bit-manipulation functions.

	/// <summary>
	/// Count the amount of high bits in a 64-bit number.
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	inline size_t count_bits(bitboard_t x) noexcept {
#if ((defined(__INTEL_COMPILER) || defined(_MSC_VER)) && defined(_WIN64)) && defined(USE_POPCNT)
		return (size_t)_mm_popcnt_u64(x);
#elif (defined(__GNUC__) && defined(USE_POPCNT))
		return __builtin_popcountll(x);
#else
		static constexpr uint64_t m1 = 0x5555555555555555;
		static constexpr uint64_t m2 = 0x3333333333333333;
		static constexpr uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
		static constexpr uint64_t h01 = 0x0101010101010101;

		x -= (x >> 1) & m1;
		x = (x & m2) + ((x >> 2) & m2);
		x = (x + (x >> 4)) & m4;
		return (x * h01) >> 56;
#endif
	}

	inline size_t scan_reverse(bitboard_t bb) { // Find the MS1B
		assert(bb != 0);

#if defined(__GNUC__) // GCC
		return size_t(63 ^ __builtin_clzll(bb));
#elif defined(_MSC_VER)

#if defined(_WIN64) // Windows 64-bit
		unsigned long idx;
		_BitScanReverse64(&idx, bb);
		return (size_t)idx;
#else // Windows 32-bit
		unsigned long idx;

		if (bb >> 32) {
			_BitScanReverse(&idx, int32_t(bb >> 32));
			return size_t(idx + 32);
		}
		else {
			_BitScanReverse(&idx, int32_t(bb));
			return size_t(idx);
		}
#endif

#else // Other
		static constexpr int index64[64] = {
			0, 47,  1, 56, 48, 27,  2, 60,
			57, 49, 41, 37, 28, 16,  3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11,  4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30,  9, 24,
			13, 18,  8, 12,  7,  6,  5, 63
		};

		const bitboard_t debruijn64 = 0x03f79d71b4cb0a89;
		bb |= bb >> 1;
		bb |= bb >> 2;
		bb |= bb >> 4;
		bb |= bb >> 8;
		bb |= bb >> 16;
		bb |= bb >> 32;
		return index64[(bb * debruijn64) >> 58];
#endif
	}

	inline size_t scan_forward(bitboard_t bb) { // Find the LS1B
		assert(bb != 0);

#if defined(__GNUC__) // GCC intrinsic.
		return size_t(__builtin_ctzll(bb));
#elif defined(_MSC_VER)

#if defined(_WIN64) // Windows 64-bit
		unsigned long idx;
		_BitScanForward64(&idx, bb);
		return (size_t)idx;
#else  // Windows 32-bit
		unsigned long idx;

		if (bb & 0xffffffff) {
			_BitScanForward(&idx, int32_t(bb));
			return size_t(idx);
		}
		else {
			_BitScanForward(&idx, int32_t(bb >> 32));
			return size_t(idx + 32);
		}
#endif

#else // Other OS/compiler
		static constexpr int index64[64] = {
			0, 47,  1, 56, 48, 27,  2, 60,
			57, 49, 41, 37, 28, 16,  3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11,  4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30,  9, 24,
			13, 18,  8, 12,  7,  6,  5, 63
		};

		const bitboard_t debruijn64 = 0x03f79d71b4cb0a89;
		return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
#endif
	}

	// Find the least significant 1 bit, set it to zero and return the placement.
	inline size_t pop_bit(bitboard_t& bb) noexcept {
		auto i = scan_forward(bb);
		bb ^= (uint64_t(1) << i);
		return i;
	}

	// Shifts the bitboard one square in some direction. From white's perspective.
	template <DIRECTION d>
	constexpr inline bitboard_t shift(bitboard_t bb) {
		return d == NORTH ? bb << 8
			: d == SOUTH ? bb >> 8
			: d == EAST ? (bb & ~bitmasks::file_masks[FILE_H]) << 1
			: d == WEST ? (bb & ~bitmasks::file_masks[FILE_A]) >> 1
			: d == NORTHEAST ? (bb & ~(bitmasks::file_masks[FILE_H] | bitmasks::rank_masks[RANK_8])) << 9
			: d == NORTHWEST ? (bb & ~(bitmasks::file_masks[FILE_A] | bitmasks::rank_masks[RANK_8])) << 7
			: d == SOUTHEAST ? (bb & ~(bitmasks::file_masks[FILE_H] | bitmasks::rank_masks[RANK_1])) >> 7
			: d == SOUTHWEST ? (bb & ~(bitmasks::file_masks[FILE_A] | bitmasks::rank_masks[RANK_1])) >> 9
			: 0;
	}

	// Other commonly used functions.

	inline constexpr RANK rank(SQUARE sq) noexcept {
		return static_cast<RANK>(sq / 8);
	}

	inline constexpr FILE file(SQUARE sq) noexcept {
		return static_cast<FILE>(sq % 8);
	}

	inline constexpr SQUARE get_square(RANK r, FILE f) noexcept {
		return static_cast<SQUARE>((r * 8) + f);
	}

	// Simple overload.
	inline constexpr SQUARE get_square(int64_t r, int64_t f) noexcept {
		return get_square(
			static_cast<RANK>(r),
			static_cast<FILE>(f));
	}
}


namespace loki::movegen {
	// Constants and type declarations.
	enum move_type : size_t {
		CAPTURES,
		QUIET,
		ALL
	};

	template<size_t _Size>
	class move_stack;
	class move_generator;

	template<size_t _Size>
	using move_stack_t = std::unique_ptr<move_stack<_Size>>;
	using move_generator_t = std::unique_ptr<move_generator>;

	namespace magics {
		class slider_generator;

		using slider_generator_t = std::shared_ptr<slider_generator>;
	}
}

namespace loki::position {
	class castling_rights;
	struct game_state;
	class position;

	using game_state_t		= std::unique_ptr<game_state>;
	using position_t		= std::shared_ptr<position>;
	using weak_position_t	= std::weak_ptr<position>;
}

#include "position/castling_rights.h"
#include "position/gamestate.h"
#include "position/position.h"

#include "movegen/magics/magics_index.h"
#include "movegen/magics/slider_generator.h"
#include "movegen/move_stack.h"
#include "movegen/move_list.h"
#include "movegen/move_generator.h"

#endif