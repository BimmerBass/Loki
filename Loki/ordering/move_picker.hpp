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
#include "search/stats.hpp"
#include <optional>

namespace loki::ordering
{
	// TODO: Collect move ordering statistics during search and pass them to this move picker object.
	template<movegen::move_type MT>
	class move_picker final
	{
		CHILD_EXCEPTION(ordering_error, loki_exception);

		inline static constexpr uint16_t HISTORY_MAX = 10'000;
		inline static constexpr uint16_t KILLER_TWO = HISTORY_MAX + 1;
		inline static constexpr uint16_t KILLER_ONE = KILLER_TWO + 1;
		static_assert(KILLER_TWO <= 0x7FFF);

	private:
		const search::search_statistics& statistics;
		const position::search_position& position;
		movegen::move_list moves;
		movegen::move_list::it current_move_it;

	public:
		move_picker(
			const position::search_position& pos,
			const search::search_statistics& stats)
			: position{ pos }, statistics{ stats }, moves{}, current_move_it{}
		{}

		/// <summary>
		/// Generate and score the moves.
		/// </summary>
		void generate_moves()
		{
			if (moves.size() != 0)
				throw_msg<ordering_error>("cannot call generate_moves more than once.");
			position.generate_moves<MT>(&moves);
			current_move_it = moves.begin();

			// TODO: Score moves in move_generator instead of here!!!
			const auto& killers = statistics.killer_moves[position.ply()];
			for (auto& move : moves)
			{
				if (!move.is_active())
				{
					if (move.get_move() == std::get<0>(killers))
						move.score(KILLER_ONE);
					else if (move.get_move() == std::get<1>(killers))
						move.score(KILLER_TWO);
				}
			}
		}

		/// <summary>
		/// Pick the next move to search.
		/// This move is picked using selection sort by looking through all the yet un-searched moves and placing
		/// the best one at the current element of the list.
		/// </summary>
		/// <returns>The next move, if there are any left.</returns>
		std::optional<movegen::move> get_next_move() noexcept
		{
			if (current_move_it == moves.end())
				return std::nullopt;


			// explicitly copy the iterator
			auto best_it = current_move_it;
			auto best_score = current_move_it->score();


			// again, copy
			for (auto it = std::next(current_move_it); it != moves.end(); ++it)
			{
				if (it->score() > best_score)
				{
					best_it = it;
					best_score = it->score();
				}
			}

			// swap the placements.
			std::iter_swap(current_move_it, best_it);
			return *current_move_it++;
		}
	};
}