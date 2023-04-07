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
#include "loki.pch.hpp"
using namespace loki::movegen;
using namespace loki::ordering;

namespace loki::search
{
	/// <summary>
	/// Basically just alpha_beta without any pruning or aggressive reductions
	/// </summary>
	eValue searcher::root_search(eDepth depth, eValue alpha, eValue beta)
	{
		assert(depth > 0);
		m_stats->info.nodes++;
		m_stats->pvTable->reset_for_ply(m_pos->ply());

		// Transposition table probing. For a root search we will only use this for move ordering.
		move_t ttMove = MOVE_NULL;
		eValue ttScore = VALUE_ZERO;
		eDepth ttDepth = ZERO_DEPTH;
		ttFlag ttFlag = NO_FLAG;
		m_hashTable->probe(m_pos->position_hash(), m_pos->ply(), ttMove, ttScore, ttDepth, ttFlag);

		// Setup a move_sorter responsible for generating and scoring moves, while also picking the best one for us.
		auto sorter = move_sorter::make_regular_scorer(m_pos, m_stats, ttMove);
		
		auto move = MOVE_NULL, best_move = MOVE_NULL;
		size_t legal = 0, moves_searched = 0;
		auto new_depth = depth;
		auto score = -VALUE_INF, best_score = -VALUE_INF;
		auto raised_alpha = false;

		while (move = sorter.get_next())
		{
			if (!m_pos->make_move(move))
				continue;
			legal++;

			// TODO: Re-implement PVS (Principal-Variation-Search) here
			score = -alpha_beta(new_depth - 1, -beta, -alpha);

			m_pos->undo_move();

			if (score >= beta) // Fail high
			{
				m_stats->info.fail_high++;
				if (legal == 1)
					m_stats->info.fail_high_first++;

				// Even though it seems a bit silly applying killer moves in the root, it makes sense if Loki ever gets aspiration windows, since identifying a window
				// that is too narrow will likely go faster.
				if (m_pos->type_of(move) == QUIET)
					m_stats->update_quiet_heuristics(m_pos, move, m_pos->ply());
				m_hashTable->store(m_pos->position_hash(), m_pos->ply(), move, beta, depth, FLAG_BETA);

				return beta;
			}

			if (score > best_score)
			{
				best_score = score;
				best_move = move;

				if (score > alpha)
				{
					alpha = score;
					raised_alpha = true;

					m_stats->pvTable->update_pv(m_pos->ply(), best_move);
				}
			}

			if (m_stop->load(std::memory_order_relaxed))
				return VALUE_ZERO;
		}

		if (legal == 0)
		{
			if (m_pos->in_check())
				return -VALUE_INF + m_pos->ply();
			else
				return VALUE_ZERO;
		}

		if (raised_alpha)
			m_hashTable->store(m_pos->position_hash(), m_pos->ply(), best_move, alpha, depth, FLAG_EXACT);
		else
			m_hashTable->store(m_pos->position_hash(), m_pos->ply(), best_move, alpha, depth, FLAG_ALPHA);
		
		return alpha;
	}
}