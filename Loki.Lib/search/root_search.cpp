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

namespace loki::search
{
	/// <summary>
	/// Basically just alpha_beta without any pruning or aggressive reductions
	/// </summary>
	eValue searcher::root_search(eDepth depth, eValue alpha, eValue beta)
	{
		assert(depth > 0);
		m_info.nodes++;
		m_pvTable.reset_for_ply(m_pos->ply());

		// Generate all pseudo-legal moves and traverse the list.
		const move_list_t& moves = m_pos->generate_moves();
		auto best_move = MOVE_NULL;

		size_t legal = 0, moves_searched = 0;
		auto new_depth = depth;
		auto score = -VALUE_INF, best_score = -VALUE_INF;
		auto raised_alpha = false;

		for (const move_list_t::scored_move& sm : moves)
		{
			auto move = sm.move;

			if (!m_pos->make_move(move))
				continue;
			legal++;

			if (legal == 1)
				score = -alpha_beta(new_depth - 1, -beta, -alpha);
			else
			{
				score = -alpha_beta(new_depth - 1, -(alpha + 1), -alpha);

				if (score > alpha) // for fail-soft: && score < beta
					score = -alpha_beta(new_depth - 1, -beta, -alpha);
			}
			m_pos->undo_move();

			if (score >= beta) // Fail high
			{
				m_info.fail_high++;
				if (legal == 1)
					m_info.fail_high_first++;

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

					m_pvTable.update_pv((eDepth)m_pos->ply(), best_move);
				}
			}

			if (m_stop)
				return VALUE_ZERO;
		}

		if (legal == 0)
		{
			if (m_pos->in_check())
				return -VALUE_INF + m_pos->ply();
			else
				return VALUE_ZERO;
		}
		return alpha;
	}
}