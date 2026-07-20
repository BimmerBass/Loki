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
#include "move_scores.hpp"
#include <optional>

namespace loki::ordering
{
	// TODO: Collect move ordering statistics during search and pass them to this move picker object.
	template<movegen::move_type MT>
	class move_picker final
	{
		CHILD_EXCEPTION(ordering_error, loki_exception);
	private:
		const search::search_statistics& statistics;
		const position::search_position& position;
		movegen::move_list moves;
		movegen::move_list::it current_move_it;

		movegen::move_list _searched_quiets;
	public:
		move_picker(
			const position::search_position& pos,
			const search::search_statistics& stats)
			: position{ pos }, statistics{ stats }, moves{}, current_move_it{}, _searched_quiets{}
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
				if (move.is_active() && position.move_is_capture(move))
					move.score(score_capture(move));
				else
				{
					if (move.get_move() == std::get<0>(killers))
						move.score(KILLER_ONE);
					else if (move.get_move() == std::get<1>(killers))
						move.score(KILLER_TWO);
					else
						move.score(statistics.history_score(position.side_to_move(), move));
				}
			}

			_searched_quiets.clear();
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

		const movegen::move_list& searched_quiets() const { return _searched_quiets; }
		movegen::move_list& searched_quiets() { return _searched_quiets; }

	private:
		movegen::move_score_t score_capture(const movegen::move& move) const noexcept
		{
			assert(move.is_active() && position.move_is_capture(move));

			// indexed by captured piece and capturer
			static util::nested_array_t<movegen::move_score_t, NUM_PIECES, NUM_PIECES> mvvlva = { {
				{ 10, 9, 9, 8, 7, 6}, // pawn: attackers: PNBRQK
				{ 20, 19, 19, 18, 17, 16 }, // knight
				{ 20, 19, 19, 18, 17, 16 }, // bishop
				{ 30, 29, 29, 28, 27, 26 }, // rook
				{ 40, 39, 39, 38, 37, 36 }, // queen
				{ -GOOD_CAPTURE * 2, -GOOD_CAPTURE * 2,-GOOD_CAPTURE * 2,-GOOD_CAPTURE * 2,-GOOD_CAPTURE * 2,-GOOD_CAPTURE * 2} // king -> zero: obviously illegal so bad capture
			} };
			auto view = position.make_view();

			auto stm = position.side_to_move();
			auto piece_captured = view.game_state()->piece_placements[!stm][move.to()];
			if (move.type() == movegen::ENPASSANT)
				piece_captured = PAWN;
			
			auto piece_capturing = view.game_state()->piece_placements[stm][move.from()];
			if (move.type() == movegen::PROMOTION)
				piece_capturing = move.promotion_piece();
			return GOOD_CAPTURE + mvvlva[piece_captured][piece_capturing];
		}
	};
}