#pragma once
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
#include "position/game_state.hpp"
#include "movegen/move.hpp"
#include "movegen/magics/magic_index.hpp"

#include <optional>
#include <vector>

namespace loki
{
	class loki_engine final
	{
	public:
		loki_engine();

		//void reset_state();
		//void set_position(std::string fen);
		bool set_position(const position::game_state& state, const std::vector<movegen::move>& moves);

		const position::game_state& state() const
		{
			if (!game_state.has_value())
				throw_msg<loki_exception>("game state has not been initialized.");
			return game_state.value();
		}
	private:
		const movegen::magics::magic_index_t rook_index;
		const movegen::magics::magic_index_t bishop_index;

		std::optional<position::game_state> game_state;
	};

}
