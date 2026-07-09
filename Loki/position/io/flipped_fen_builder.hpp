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
#include "base_builder.hpp"
#include "game_state_builder.hpp"

namespace loki::position::io
{
	class flipped_fen_builder : public base_builder<std::string, std::string>
	{
	public:
		using bb_t = base_builder<std::string, std::string>;

	private:
		game_state_builder::splitted_fen splitted;
	protected:
		void reset_internal() override;

	public:
		virtual bb_t& piece_placements() override;
		virtual bb_t& side_to_move() override;
		virtual bb_t& castling_ability() override;
		virtual bb_t& en_passant_square() override;
		virtual bb_t& halfmove_clock() override;
		virtual bb_t& fullmove_clock() override;
	};
}
