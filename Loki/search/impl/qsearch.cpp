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

#include "../search_worker.hpp"
#include "ordering/move_picker.hpp"

using namespace loki;
using namespace loki::search;
using namespace loki::movegen;
using namespace loki::position;
using namespace loki::ordering;

template<side S, move_type MT> requires (S < NUM_SIDES&& MT != movegen::QUIET)
score_t search_worker::qsearch(
	search_position& position,
	score_t alpha,
	score_t beta)
{
	// evaluator.evaluate<S>(position.make_view());
	assert(S == position.side_to_move());
	assert(alpha >= -constants::SCORE_INF && alpha < beta && beta <= constants::SCORE_INF);

	if (position.is_draw())
			return constants::SCORE_ZERO;

	// protect internal structures
	if (position.ply() >= MAX_PLY)
    	return evaluator.evaluate<S>(position.make_view());
	
	// if we're in check, generate all moves since we would otherwise return false mate scores
	const bool in_check = position.in_check();
	if constexpr (MT == ACTIVE)
	{
		if (in_check)
			return qsearch<S, ALL>(position, alpha, beta);

		// stand-pat.
		auto evaluation = evaluator.evaluate<S>(position.make_view());

		if (evaluation >= beta)
			return evaluation;
		if (evaluation > alpha)
			alpha = evaluation;
	}

	statistics.selective_depth = std::max(statistics.selective_depth, (size_t)position.ply());
	statistics.nodes++;

	size_t legal_moves = 0;
	move_picker<MT> mp(position, statistics);
	mp.generate_moves();
	std::optional<movegen::move> move_opt;

	while ((move_opt = mp.get_next_move()) != std::nullopt)
	{
		const auto move = move_opt.value();
		if (!position.make_move(move))
			continue;

		legal_moves++;
		auto score = -qsearch<!S, ACTIVE>(position, -beta, -alpha);
		position.undo_last_move();

		if (score > alpha)
			alpha = score;
		if (score >= beta)
		{
			statistics.fail_high++;
			if (legal_moves == 1)
				statistics.fail_high_first_move++;
			return score;
		}
	}

	// checkmate if in check
	if constexpr (MT == ALL)
	{
		assert(in_check);
		if (legal_moves == 0)
			return mate_in(position.ply());
	}

	return alpha;
}

template score_t search_worker::qsearch<WHITE, ALL>(search_position&, score_t, score_t);
template score_t search_worker::qsearch<WHITE, ACTIVE>(search_position&, score_t, score_t);
template score_t search_worker::qsearch<BLACK, ALL>(search_position&, score_t, score_t);
template score_t search_worker::qsearch<BLACK, ACTIVE>(search_position&, score_t, score_t);