/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef DEFS_H
#define DEFS_H

#include <iostream>
#include <assert.h>


#if (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#include <xmmintrin.h> // Used for _mm_prefetch
#endif


constexpr int MAXPOSITIONMOVES = 256;
constexpr int MAXGAMEMOVES = 1024;
constexpr int NOMOVE = 0;
constexpr int MOVE_NULL = 0;


const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";




enum square : int {
	A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
	A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
	A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
	A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
	A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
	A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
	A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
	A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63, NO_SQ = 65
};


enum SIDE :int { WHITE = 1, BLACK = 0 };
enum piece :int { PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5, NO_TYPE = 6 };
constexpr int decode_promo[4] = { KNIGHT, BISHOP, ROOK, QUEEN };

enum C_RIGHTS { WKCA = 0, WQCA = 1, BKCA = 2, BQCA = 3 };

enum rank : int { RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3, RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, NO_RANK = 8 };
enum file : int { FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3, FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7, NO_FILE = 8 };

enum DIRECTION:int{NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3, NORTHWEST = 4, NORTHEAST = 5, SOUTHWEST = 6, SOUTHEAST = 7};

constexpr int MAXDEPTH = 100;
constexpr int INF = 40000;
constexpr int MATE = INF - MAXDEPTH;
constexpr int VALUE_NONE = 50000;


// Used to allocate size x in megabytes of caches.
#define KB(x) (x << 10)
#define MB(x) (x << 20)

// Boundaries and default size of transposition table
#define TT_MAX_SIZE 1000
#define TT_DEFAULT_SIZE 16
#define TT_MIN_SIZE 1


// Amount of threads to use
#define THREADS_MAX_NUM 8
#define THREADS_DEFAULT_NUM 1
#define THREADS_MIN_NUM 1

typedef uint64_t Bitboard;


#endif