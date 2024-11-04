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
#include "io/game_state_builder.hpp"


namespace loki::position
{
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


		inline static game_state_t from_fen(const std::string& fen)
		{
			io::game_state_builder builder;
			return from_builder(&builder, fen);
		}

		// this is the actual method we will test.
		inline static game_state_t from_builder(io::base_builder<std::string, game_state>* bPtr, const std::string& fen)
		{
			auto gs = std::make_shared<game_state>();
			auto sPtr = std::shared_ptr<std::string>(new std::string(fen));
			bPtr->reset(sPtr, gs);
			return (*bPtr)
				.piece_placements()
				.side_to_move()
				.castling_ability()
				.en_passant_square()
				.halfmove_clock()
				.fullmove_clock()
				.get_product();
		}
	};
}