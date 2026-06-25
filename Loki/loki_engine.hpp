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
#include "position/search_position.hpp"
#include "movegen/move.hpp"
#include "movegen/magics/magic_index.hpp"

#include <iostream>
#include <optional>
#include <vector>

namespace loki
{
	class loki_engine final
	{
	public:
		loki_engine();

		//void reset_state();

		/// <summary>
		/// Set a position a given initial state and a list of moves to be applied
		/// </summary>
		/// <param name="state"></param>
		/// <param name="moves"></param>
		/// <returns></returns>
		bool set_position(const position::game_state& state, const std::vector<movegen::move>& moves);

		/// <summary>
		/// Set the position given a complete initial state.
		/// </summary>
		/// <param name="state"></param>
		void set_position(position::search_position_t state);

		/// <summary>
		/// Make a new position pointer using the already-initialized rook and bishop index.
		/// </summary>
		/// <param name="state"></param>
		/// <returns></returns>
		position::search_position_t make_position(const position::game_state& state) const;

		[[maybe_unused]]
		size_t perft(size_t depth, std::ostream& out) const;

		const position::search_position_t& position() const
		{
			if (!_position.has_value())
				throw_msg<loki_exception>("position has not been initialized.");
			return _position.value();
		}
	private:
		const movegen::magics::magic_index_t rook_index;
		const movegen::magics::magic_index_t bishop_index;

		std::optional<position::search_position_t> _position;
	};

}
