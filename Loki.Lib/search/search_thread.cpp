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

namespace loki::search
{
	search_thread::search_thread(
		const position::game_state&					state,
		const movegen::magics::slider_generator_t	slider_generator,
		evaluation::evaluation_params_t&			eval_params) 
		: m_info{ ZeroInfo }
	{
		m_pos = position::position::create_position(std::make_shared<position::game_state>(state), slider_generator);
		m_eval = std::make_unique<evaluation::evaluator>(m_pos, eval_params);
	}

	eValue search_thread::alpha_beta(eDepth depth, eValue alpha, eValue beta)
	{
		assert(beta > alpha);

		if (m_pos->ply() >= m_info.selective_depth)
			m_info.selective_depth = static_cast<eDepth>(m_pos->ply());

		// If we've reached sufficient depth, return a static evaluation.
		if (depth <= 0)
			return m_eval->score_position();

		// We've entered a new node.
		m_info.nodes++;

		if ((m_info.nodes & 2047) == 0)
			check_stopped_search();

		if (m_stop)
			return VALUE_ZERO;
		
		auto is_root = (m_pos->ply() == 0);
		auto is_pv = (beta - alpha == 1) ? false : true; // We are in a PV-node if we aren't in a null window.

		// Check for draws by 50-move rule
		if (!is_root && m_pos->is_draw())
			return VALUE_ZERO;

		if (!is_root && m_pos->ply() >= MAX_DEPTH)
			return m_eval->score_position();


		// Generate all pseudo-legal moves and traverse the list.
		auto moves = m_pos->generate_moves();
		auto best_move = MOVE_NULL;

		size_t legal = 0, moves_searched = 0;
		auto new_depth = depth;
		auto score = -VALUE_INF, best_score = -VALUE_INF;

		for (const auto& sm : moves)
		{
			auto move = sm.move;
			if (!m_pos->make_move(move)) /* move is illegal */
				continue;
			legal++; // Store amount of legal moves so as to not miss check-/stalemates.

			// Principal variation search: Always search first move with full window, and at full depth.
			new_depth = depth - 1;

			if (moves_searched == 0)
				score = -alpha_beta(new_depth, -beta, -alpha);
			else
			{
				score = -alpha_beta(new_depth, -(alpha + 1), -alpha);

				if (score > alpha && score < beta)
					score = -alpha_beta(new_depth, -beta, -alpha);
			}

			// undo the latest move after searching.
			m_pos->undo_move();
			moves_searched++;

			// If the recursive call set our stop flag, we will just return.
			if (m_stop)
				return VALUE_ZERO;

			if (score >= beta) /* Fail-hard beta cutoff */
			{
				// Update statistics
				m_info.fail_high++;
				if (moves_searched == 1)
					m_info.fail_high_first++;

				return beta;
			}
			if (score > best_score) // score higher than alpha (previously believed to be best score) i.e. new best score.
			{
				best_score = score;
				best_move = move;

				if (score > alpha)
					alpha = score;
			}
		}
		// Check-/stalemate detection
		if (legal <= 0)
		{
			if (m_pos->in_check())
				return -VALUE_INF + m_pos->ply();
			else
				return VALUE_ZERO;
		}

		// fail-hard
		return alpha;
	}

	void search_thread::check_stopped_search()
	{
		if (stop_searching->load(std::memory_order_relaxed))
			m_stop = true;
	}
	
	void search_thread::preprocess_search()
	{
		m_stop = false;
		m_info = ZeroInfo;
	}
}