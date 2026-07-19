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

#include "movegen/move.hpp"
#include "movegen/move_list.hpp"
#include "position/search_position.hpp"

namespace loki::ordering
{
	// TODO: Collect move ordering statistics during search and pass them to this move picker object.
	template<movegen::move_type MT>
	class move_picker final
	{
		CHILD_EXCEPTION(ordering_error, loki_exception);
	private:
		const position::search_position& position;
		movegen::move_list moves;
		movegen::move_list::cit current_move_it;

	public:
		/// <summary>
		/// Generate and score the moves.
		/// </summary>
		void generate_moves()
		{
			if (moves.size() != 0)
				throw_msg<ordering_error>("cannot call generate_moves more than once.");
			position.generate_moves<MT>(&moves);
			current_move_it = moves.begin();

			throw not_implemented_error();
		}

		/// <summary>
		/// Pick the next move to search.
		/// This move is picked using selection sort by looking through all the yet un-searched moves and placing
		/// the best one at the current element of the list.
		/// </summary>
		/// <returns>The next move, if there are any left.</returns>
		std::optional<move> get_next_move()
		{
			if (current_move_it == moves.end())
				return std::nullopt;
			throw not_implemented_error();
		}
	};
}