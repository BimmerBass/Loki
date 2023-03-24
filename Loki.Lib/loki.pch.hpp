//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef LOKI_PCH_H
#define LOKI_PCH_H
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <memory>
#include <array>
#include <mutex>
#include <iostream>
#include <assert.h>
#include <format>
#include <regex>
#include <random>
#include <sstream>
#include <map>

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#include <nmmintrin.h> // Used for count_bits
#if defined(_WIN64)
#include <intrin.h> // Used for scan_forward and scan_reverse.
#endif
#endif

#include "utility/exception.hpp"
#include "utility/operators.hpp"

namespace loki {
	EXCEPTION(e_lokiError);
	EXCEPTION_CLASS(e_notImplementedError, e_lokiError);

	// General type definitions.
	// Note: A move is stored in a 16-bit integer, and the data is saved as follows (from lsb to msb):
	// bit 0..5 -> to square (0 to 63)
	// bit 6..11 -> from square (0 to 63)
	// bit 12..13 -> promotion piece ((0) knight = 00, (1) bishop = 01, (2) rook = 10, (3) queen = 11)
	// bit 14..16 -> special move flag ((0) promotion = 00, (1) en-passant = 01, (2) castling = 2, (3) neither = 11)
	enum move_t : uint16_t
	{
		MOVE_NULL = 0
	};
	using bitboard_t	= uint64_t;
	using hashkey_t		= uint64_t;

	enum eSide : int64_t
	{
		WHITE = 0,
		BLACK = 1,
		SIDE_NB = 2
	};
	inline constexpr eSide operator!(eSide s)
	{
		return (s == WHITE) ? BLACK : WHITE;
	}

	enum ePiece : int64_t
	{
		PAWN = 0,
		KNIGHT = 1,
		BISHOP = 2,
		ROOK = 3,
		QUEEN = 4,
		KING = 5,
		PIECE_NB = 6,
		PIECE_NB_TOTAL = 12
	};

	enum eSquare : int64_t
	{
		A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
		A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
		A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
		A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
		A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
		A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
		A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
		A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63, SQ_NB = 64, NO_SQ = 65
	};
	ENABLE_INCR_OPERATORS_ON(eSquare);

	enum eCastlingRights : uint8_t
	{
		WKCA = 0,
		WQCA = 1,
		BKCA = 2,
		BQCA = 3
	};

	enum eRank : int64_t
	{
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
	enum eFile : int64_t
	{
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

	enum eDirection :size_t
	{
		NORTH = 0,
		SOUTH = 1,
		EAST = 2,
		WEST = 3,
		NORTHWEST = 4,
		NORTHEAST = 5,
		SOUTHWEST = 6,
		SOUTHEAST = 7
	};

	enum eDepth : size_t
	{
		MAX_DEPTH = 100
	};
	ENABLE_BASE_OPERATORS_ON(eDepth);
	ENABLE_INCR_OPERATORS_ON(eDepth);
	
	/// <summary>
	/// eValue describes the value of a given position in Loki's internal units (1 unit = 1 / 512 * pawn-value)
	/// </summary>
	enum eValue : int64_t
	{
		VALUE_ZERO = 0,
		VALUE_INF = 40000,
		VALUE_NONE = 50000,
		VALUE_MATE = VALUE_INF - static_cast<long>(MAX_DEPTH)
	};
	ENABLE_FULL_OPERATORS_ON(eValue);
	enum eCentipawnValue : int64_t {}; /* eCentipawnValue is defined so as to distinguish between Loki's internal units and the centi-pawn (one 100'th of a pawn) units that the UCI protocol requires. */


	// Constant declarations.
	constexpr size_t MAX_POSITION_MOVES		= 256; // The maximum amount of pseudo-legal moves in a given position. TODO: Remove this restraint.
	constexpr size_t MAX_GAME_MOVES			= 1024; // Maximum amount of moves in a game.  TODO: Remove this restraint.

	// Bit masks.
	namespace bitmasks
	{

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
	/// Check if a bit is set.
	/// </summary>
	/// <typeparam name="_Ty">The type of the number to check.</typeparam>
	/// <typeparam name="_IdxT">The type of the index.</typeparam>
	/// <param name="x"></param>
	/// <param name="idx"></param>
	/// <returns></returns>
	template<typename _Ty, typename _IdxT> requires(std::is_arithmetic_v<_Ty> && std::is_arithmetic_v<_IdxT>)
	inline constexpr bool is_set(const _Ty& x, _IdxT idx) noexcept
	{
		return ((x >> idx) & 1) != 0;
	}

	/// <summary>
	/// Set a bit.
	/// </summary>
	/// <typeparam name="_Ty"></typeparam>
	/// <typeparam name="_IdxT"></typeparam>
	/// <param name="x"></param>
	/// <param name="idx"></param>
	/// <returns></returns>
	template<typename _Ty, typename _IdxT> requires(std::is_arithmetic_v<_Ty>&& std::is_arithmetic_v<_IdxT>)
	inline constexpr _Ty set_bit(const _Ty& x, _IdxT idx) noexcept
	{
		return (x | (_Ty(1) << idx));
	}

	/// <summary>
	/// Toggle a bit.
	/// </summary>
	/// <typeparam name="_Ty"></typeparam>
	/// <typeparam name="_IdxT"></typeparam>
	/// <param name="x"></param>
	/// <param name="idx"></param>
	/// <returns></returns>
	template<typename _Ty, typename _IdxT> requires(std::is_arithmetic_v<_Ty>&& std::is_arithmetic_v<_IdxT>)
	inline constexpr _Ty toggle_bit(const _Ty& x, _IdxT idx) noexcept
	{
		return (x ^ (_Ty(1) << idx));
	}


	/// <summary>
	/// Count the amount of high bits in a 64-bit number.
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	inline size_t count_bits(bitboard_t x) noexcept
	{
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

	inline size_t scan_reverse(bitboard_t bb)
	{ // Find the MS1B
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

		if (bb >> 32)
		{
			_BitScanReverse(&idx, int32_t(bb >> 32));
			return size_t(idx + 32);
		}
		else
		{
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

	inline size_t scan_forward(const bitboard_t& bb)
	{ // Find the LS1B
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

		if (bb & 0xffffffff)
		{
			_BitScanForward(&idx, int32_t(bb));
			return size_t(idx);
		}
		else
		{
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
	inline size_t pop_bit(bitboard_t& bb) noexcept
	{
		auto i = scan_forward(bb);
		bb ^= (uint64_t(1) << i);
		return i;
	}

	// Shifts the bitboard one square in some direction. From white's perspective.
	template <eDirection d>
	constexpr inline bitboard_t shift(const bitboard_t& bb)
	{
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

	inline constexpr eRank rank(eSquare sq) noexcept
	{
		return static_cast<eRank>(sq / 8);
	}
	inline constexpr eRank rank(size_t sq) noexcept
	{
		return rank(static_cast<eSquare>(sq));
	}

	inline constexpr eFile file(eSquare sq) noexcept
	{
		return static_cast<eFile>(sq % 8);
	}

	inline constexpr eSquare get_square(eRank r, eFile f) noexcept
	{
		return static_cast<eSquare>((r * 8) + f);
	}

	// Simple overload.
	inline constexpr eSquare get_square(int64_t r, int64_t f) noexcept
	{
		return get_square(
			static_cast<eRank>(r),
			static_cast<eFile>(f));
	}

	/// <summary>
	/// Check whether or not a string describes a square in algebraic notation.
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	inline bool is_algebraic(std::string sq)
	{
		return (sq.size() == 2 &&
			std::string("abcdefgh").find(sq[0]) != std::string::npos &&
			std::string("12345678").find(sq[1]) != std::string::npos);
	}

	/// <summary>
	/// Convert a square to algebraic notation.
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	inline std::string to_algebraic(eSquare sq)
	{
		const static std::array<std::string, FILE_NB> file_names = { "a", "b", "c", "d", "e", "f", "g", "h" };
		size_t f = file(sq);
		size_t r = rank(sq) + 1;

		return file_names[f] + std::to_string(r);
	}

	/// <summary>
	/// Convert a string of algebraic notation to an internal square representation.
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	inline eSquare from_algebraic(std::string str)
	{
		static std::map<char, eFile> file_mappings = {
			{'a', FILE_A},
			{'b', FILE_B},
			{'c', FILE_C},
			{'d', FILE_D},
			{'e', FILE_E},
			{'f', FILE_F},
			{'g', FILE_G},
			{'h', FILE_H},
		};
		auto file = str[0];
		auto rank = str[1];

		return get_square((rank - '0') - 1, file_mappings[std::tolower(file)]);
	}
}

namespace loki::utility
{
	class perft;

	template<typename _Ty, size_t _Size> requires (_Size > 0)
		class fast_stack;

	class initializer;
}

namespace loki::movegen
{
	// Constants and type declarations.
	enum MOVE_TYPE : size_t
	{
		ACTIVES,	/* All active moves; captures, promotions etc.. */
		QUIET,		/* All quiet moves. */
		ALL			/* All moves. */
	};

	enum SPECIAL_MOVE : size_t
	{
		PROMOTION = 0,
		ENPASSANT = 1,
		CASTLE = 2,
		NOT_SPECIAL = 3
	};

	// Inline functions.
	inline constexpr eSquare from_sq(move_t move) noexcept
	{
		return static_cast<eSquare>((move >> 4) & 63);
	}
	inline constexpr eSquare to_sq(move_t move) noexcept
	{
		return static_cast<eSquare>(move >> 10);
	}
	inline constexpr SPECIAL_MOVE special(move_t move) noexcept
	{
		return static_cast<SPECIAL_MOVE>(move & 3);
	}
	inline constexpr ePiece promotion_piece(move_t move) noexcept
	{
		return static_cast<ePiece>((move >> 2) & 3);
	}
	inline constexpr move_t create_move(size_t from_sq, size_t to_sq, size_t special, size_t promotion_piece) noexcept
	{
		return static_cast<move_t>((to_sq << 10) | (from_sq << 4) | (promotion_piece << 2) | (special));
	}
	// Check if a move is in algebraic notation
	inline bool is_algebraic_move(const std::string& move)
	{
		return move.length() == 4 &&
			is_algebraic(move.substr(0, 2)) && 
			is_algebraic(move.substr(2));
	}

	template<size_t _Size> requires (_Size > 0)
		class move_stack;
	template<size_t _Size> requires(_Size > 0)
		class move_list;
	class move_generator;

	template<size_t _Size>
	using move_stack_t = std::unique_ptr<move_stack<_Size>>;
	using move_generator_t = std::unique_ptr<move_generator>;

	using move_list_t = move_list<MAX_POSITION_MOVES>;

	namespace magics
	{
		class slider_generator;

		using slider_generator_t = std::shared_ptr<slider_generator>;
	}
}

namespace loki::position
{
	class castling_rights;
	class zobrist;
	struct game_state;
	class position;

	using zobrist_t = std::unique_ptr<zobrist>;
	using game_state_t = std::shared_ptr<game_state>;
	using position_t = std::shared_ptr<position>;
	using weak_position_t = std::weak_ptr<position>;
}

namespace loki::evaluation
{
	class evaluation_params;
	using evaluation_params_t = std::shared_ptr<evaluation_params>;

	class restricted_t
	{
		template<typename _Ty, typename... _TArgs> requires std::is_base_of_v<evaluation_params, _Ty>
		friend evaluation_params_t make_params(_TArgs&&...);

		restricted_t() = default;
	};
}

#include "position/castling_rights.hpp"
#include "position/hashing/zobrist.hpp"
#include "position/gamestate.hpp"
#include "position/position.hpp"

#include "movegen/magics/magics_index.hpp"
#include "movegen/magics/slider_generator.hpp"
#include "movegen/move_stack.hpp"
#include "movegen/move_list.hpp"
#include "movegen/move_generator.hpp"

#include "utility/fast_stack.hpp"
#include "utility/perft.hpp"
#include "utility/textutil.hpp"

#include "evaluation/parameters/evaluation_params.hpp"
#include "evaluation/evaluation.hpp"

#include "search/limits.hpp"
#include "search/search_thread.hpp"
#include "search/search_context.hpp"

#include "uci/uci.hpp"

#endif