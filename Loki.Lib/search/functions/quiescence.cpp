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
using namespace loki::ordering;

namespace loki::search
{
	/// <summary>
	/// Capture resolving to avoid the horizon effect.
	/// </summary>
	eValue searcher::quiescence(eValue alpha, eValue beta)
	{
		assert(beta > alpha);
		m_stats->info.nodes++;
		if ((m_stats->info.nodes & 2047) == 0)
			check_stopped_search();
		if (m_stop->load(std::memory_order_relaxed))
			return VALUE_ZERO;

		// Check if we've been ordered to stop, or if the position is drawn.
		if ((m_pos->is_draw() && m_pos->ply() > 0) || m_stop->load(std::memory_order_relaxed))
			return VALUE_ZERO;

		// Protect our internal structures from invalid access.
		if (m_pos->ply() >= MAX_DEPTH)
			return m_eval->score_position();

		// This is the central point of quiescence search: We evaluate the position before searching a node further, and return if
		// it already beats beta.
		auto stand_pat = m_eval->score_position();

		if (stand_pat >= beta)
			return beta;
		if (stand_pat > alpha)
			alpha = stand_pat;

		move_sorter sorter(m_pos, m_stats, true);
		auto score = -VALUE_INF;
		auto move = MOVE_NULL;
		size_t legal = 0;

		while (move = sorter.get_next())
		{
			if (!m_pos->make_move(move))
				continue;
			legal++;

			score = -quiescence(-beta, -alpha);
			m_pos->undo_move();

			if (m_stop->load(std::memory_order_relaxed))
				return VALUE_ZERO;

			if (score >= beta)
			{
				m_stats->info.fail_high++;
				if (legal == 1)
					m_stats->info.fail_high_first++;
				return beta;
			}
			if (score > alpha)
				alpha = score;
		}
		return alpha;
	}
}