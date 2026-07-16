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
#include <variant>
#include <expected>

#include "position/bitboard.hpp"
#include "util/stringops.hpp"
#include "util/operators.hpp"

// This file contains the most common type and constant definitions in Loki.
namespace loki
{
	// score and depth type for readability
	using score_t = std::int32_t;
	using depth_t = std::size_t;

	namespace constants
	{
		constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		constexpr size_t MAX_POSITION_MOVES = 256; // TODO: Remove this restriction
		constexpr size_t MAX_GAME_MOVES = 1024; // TODO: Remove this restriction

		constexpr score_t SCORE_INF = 1'000'000;
		constexpr score_t SCORE_MATE = SCORE_INF - MAX_GAME_MOVES;
		constexpr depth_t MAX_DEPTH = 100;
	}

	enum ply_t : depth_t
	{
		ROOT_PLY = 0,
		MAX_PLY = constants::MAX_GAME_MOVES
	};
	ENABLE_BASE_OPERATORS_ON(ply_t);

	inline constexpr score_t mate_in(ply_t ply) noexcept
	{
		return -constants::SCORE_INF + (score_t)ply;
	}
	inline bool is_mate(score_t score) noexcept
	{
		return std::abs(score) > constants::SCORE_MATE;
	}

	struct mate_score
	{
		mate_score(score_t s)
		{
			assert(is_mate(s));
			const auto plies = constants::SCORE_INF - std::abs(s);
			const auto moves = (plies + 1) / 2;
			in_moves = s > 0 ? moves : -moves;
		}
		int in_moves;
	};
	struct cp_score { score_t cp; };

	using search_score_t = std::variant<mate_score, cp_score>;
	using search_result_t = std::expected<search_score_t, std::exception_ptr>;


	enum side
	{
		WHITE = 0,
		BLACK,
		NUM_SIDES
	};
	ENABLE_INCR_OPERATORS_ON(side);
	inline constexpr side operator!(side s)
	{
		return s == WHITE ? BLACK : WHITE;
	}

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
	ENABLE_BASE_OPERATORS_ON(piece);
	ENABLE_INCR_OPERATORS_ON(piece);
	ENABLE_STRINGIFY(piece, "P", "N", "B", "R", "Q", "K", "NP", "-");
}