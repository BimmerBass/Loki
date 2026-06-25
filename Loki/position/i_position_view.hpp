// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include "defs.hpp"
#include "position/game_state.hpp"

namespace loki::position
{
	class i_position_view
	{
	public:
		virtual bitboard_t piece_bb(side s, piece p) const = 0;
		virtual bitboard_t all_pieces(side s) const = 0;
		virtual bitboard_t all_pieces() const = 0;
		virtual e_square king_square(side s) const = 0;
		virtual const game_state* game_state() const = 0;

		virtual ~i_position_view() {}
	};
}