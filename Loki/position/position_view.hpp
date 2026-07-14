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

	template<class T>
	concept position_view =
		requires(const T& pos, side s, piece p)
	{
		{ pos.piece_bb(s, p) } noexcept -> std::same_as<position::bitboard_t>;
		{ pos.all_pieces(s) } noexcept -> std::same_as<position::bitboard_t>;
		{ pos.all_pieces() } noexcept -> std::same_as<position::bitboard_t>;
		{ pos.king_square(s) } noexcept -> std::same_as<position::e_square>;
		{ pos.game_state() } noexcept -> std::same_as<const position::game_state*>;
	};
}
