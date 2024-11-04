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
#include <string>
#include "defs.hpp"
#include "base_builder.hpp"

namespace loki::position::io
{
	class fen_string_builder : public base_builder<game_state, std::string>
	{
		using bb_t = base_builder<game_state, std::string>;
	public:
		base_builder& piece_placements() override;
		base_builder& side_to_move() override;
		base_builder& castling_ability() override;
		base_builder& en_passant_square() override;
		base_builder& halfmove_clock() override;
		base_builder& fullmove_clock() override;

	protected:
		void reset_internal() override;
	};
}
