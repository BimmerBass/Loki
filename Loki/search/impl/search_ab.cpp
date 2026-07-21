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
#include <algorithm>
#include "ordering/move_picker.hpp"
#include "../search_worker.hpp"

using namespace loki::ordering;
using namespace loki::movegen;

namespace loki::search
{
	template<side S, search_worker::node Node> requires (S < NUM_SIDES)
	score_t search_worker::search_ab(
		position::search_position& position,
		const limits& limits,
		std::stop_token stop_token,
		size_t depth,
		score_t alpha,
		score_t beta)
	{
		assert(S == position.side_to_move());
		
		limits.check_stopping_conditions(statistics.nodes);
		if (stop_token.stop_requested())
			return constants::SCORE_ZERO;

		statistics.pv_table.reset_for_ply(position.ply());
		statistics.selective_depth = std::max(statistics.selective_depth, (size_t)position.ply());
		statistics.nodes++;

		if constexpr (Node == node::INTERNAL)
		{
			if (position.is_draw())
				return constants::SCORE_ZERO;
		}

		if (depth == 0)
		{
			return qsearch<S>(position, alpha, beta);
		}

		size_t legal_moves = 0;
		move_picker<ALL> mp(position, statistics);
		mp.generate_moves();

		auto best_score = -constants::SCORE_INF;

		std::optional<movegen::move> move_opt;
		while ((move_opt = mp.get_next_move()) != std::nullopt)
		{
			const auto move = move_opt.value();
			if constexpr (Node == node::ROOT)
			{
				auto move_allowed =
					limits.searchmoves.empty() ||
					std::ranges::find(limits.searchmoves, move) != limits.searchmoves.end();
				if (!move_allowed)
					continue;
			}

			if (!position.make_move(move))
				continue;

			legal_moves++;
			auto score = -search_ab<!S, node::INTERNAL>(position, limits, stop_token, depth - 1, -beta, -alpha);
			position.undo_last_move();

			if (stop_token.stop_requested())
				return constants::SCORE_ZERO;

			best_score = std::max(best_score, score);

			if (score > alpha)
			{
				alpha = score;
				statistics.pv_table.update_pv(position.ply(), move);
			}

			if (score >= beta)
			{
				statistics.fail_high++;
				if (legal_moves == 1) // first move
					statistics.fail_high_first_move++;

				if (!move.is_active())
				{
					auto& killer_entry = statistics.killer_moves[position.ply()];
					if (std::get<0>(killer_entry) != move.get_move())
					{
						std::get<1>(killer_entry) = std::get<0>(killer_entry);
						std::get<0>(killer_entry) = move.get_move();
					}

					// reward good quiet, penalize bad ones.
					const int32_t history_bonus = depth * depth;
					statistics.update_history<S>(move, history_bonus);
					for (const auto& prev_quiet : mp.searched_quiets())
						statistics.update_history<S>(prev_quiet, -history_bonus);
				}
				return best_score;
			}

			// save non-cutoff quiet move
			if (!move.is_active())
			{
				mp.searched_quiets().push_back(move);
			}
		}

		if (legal_moves == 0)
		{
			if (position.in_check())
				return mate_in(position.ply());
			return constants::SCORE_ZERO;
		}

		return best_score;
	}

	template score_t search_worker::search_ab<WHITE, search_worker::node::ROOT>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	template score_t search_worker::search_ab<WHITE, search_worker::node::INTERNAL>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	template score_t search_worker::search_ab<BLACK, search_worker::node::ROOT>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	template score_t search_worker::search_ab<BLACK, search_worker::node::INTERNAL>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
}
