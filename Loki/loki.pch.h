#ifndef LOKI_PCH_H
#define LOKI_PCH_H
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <iostream>

namespace loki {
	// General type definitions.
	using move_t		= uint16_t;
	using bitboard_t	= uint64_t;

	enum SIDE : size_t {
		WHITE = 0,
		BLACK = 1,
		SIDE_NB = 2
	};

	enum PIECE : size_t {
		PAWN = 0,
		KNIGHT = 1,
		BISHOP = 2,
		ROOK = 3,
		QUEEN = 4,
		KING = 5,
		PIECE_NB = 6,
		PIECE_NB_TOTAL = 12
	};

	enum SQUARE : size_t {
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

	// Constant declarations.
	constexpr size_t MAX_POSITION_MOVES		= 256; // The maximum amount of pseudo-legal moves in a given position. TODO: Remove this restraint.
	constexpr size_t MAX_GAME_MOVES			= 1024; // Maximum amount of moves in a game.  TODO: Remove this restraint.

}


namespace loki::movegen {

	template<size_t _Size>
	class move_stack;
	class move_generator;

	template<size_t _Size>
	using move_stack_t = std::unique_ptr<move_stack<_Size>>;
	using move_generator_t = std::unique_ptr<move_generator>;
}

#include "movegen/move_stack.h"


namespace loki::position {
	class castling_rights;
	struct game_state;

	using game_state_t = std::unique_ptr<game_state>;
}

#include "position/castling_rights.h"
#include "position/gamestate.h"
#include "position/position.h"



#endif