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
#include <concepts>
#include "castle_rights.hpp"
#include "defs.hpp"
#include "square.hpp"
#include "util/exception.hpp"
#include "io/base_builder.hpp"

namespace loki::position
{
	struct game_state;
	using game_state_t = std::shared_ptr<game_state>;

	/// <summary>
	/// game_state represents the most basic chess position.
	/// It acts like a DTO (data-transfer object) between internal types and FEN's and is not optimized for quick move generation.
	/// </summary>
	struct game_state
	{
		piece piece_placements[NUM_SIDES][NUM_SQUARES];
		side side_to_move;
		size_t fifty_move_cnt, full_move_cnt;
		square en_passant_sq;
		castle_rights castling_rights;

		/// <summary>
		/// Get the piece type on a given square.
		/// NOTE: This method is not optimized for speed and should therefore not be used during move generation.
		/// </summary>
		/// <param name="sq">The square in question</param>
		/// param name="sPtr">An optional pointer to a side-enum. If NULL, it is not used</param>
		/// <returns>A piece enum representing the piece, if present. Otherwise NO_PIECE</returns>
		piece get_piece(square sq, side* sPtr = nullptr) const;

		/// <summary>
		/// Generate a FEN string for the current position.
		/// </summary>
		/// <param name="gs">A shared pointer to the object.</param>
		/// <returns>A string representing the position.</returns>
		static std::string to_fen(const game_state_t& gs);

		/// <summary>
		/// Parse a FEN string to a game_state instance.
		/// </summary>
		/// <param name="fen">The string to parse</param>
		/// <returns>A std::shared_ptr to the game_state instance.</returns>
		static game_state_t from_fen(const std::string& fen);

		/// <summary>
		/// Parse a FEN string using a builder.
		/// </summary>
		/// <param name="bPtr">Pointer to the builter to use</param>
		/// <param name="fen">The string to parse.</param>
		/// <returns>A std::shared_ptr to the game_state instance.</returns>
		static game_state_t from_builder(io::base_builder<std::string, game_state>* bPtr, const std::string& fen);
	};
}