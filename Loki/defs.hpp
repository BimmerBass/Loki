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
#pragma once
#include <cstdint>
#include <cassert>
#include <memory>

#include "position/bitboard.hpp"
#include "util/stringops.hpp"

// This file contains the most common type and constant definitions in Loki.
namespace loki
{
	constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	
	enum side {
		WHITE = 0,
		BLACK,
		NUM_SIDES
	};
	ENABLE_STRINGIFY(side, "W", "B", "-");
	enum piece
	{
		PAWN = 0,
		KNIGHT,
		BISHOP,
		ROOK,
		QUEEN,
		KING,
		NUM_PIECES,
		NO_PIECE
	};
	ENABLE_STRINGIFY(piece, "P", "N", "B", "R", "Q", "K", "NP", "-");


	namespace position
	{
		struct game_state;
		using game_state_t = std::shared_ptr<game_state>;
	}
}