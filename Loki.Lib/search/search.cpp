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
#include "utility/check_input.hpp"

using namespace loki::movegen;

namespace loki::search
{
	/// <summary>
	/// Initialize an empty searcher, and make sure the parameters are valid.
	/// </summary>
	searcher::searcher(
		eThreadId threadId,
		const movegen::magics::slider_generator_t& sliderGen,
		const evaluation::evaluation_params_t& params,
		ttPtr_t& hashTable, std::shared_ptr<std::atomic_bool> stop)
		: m_threadId(threadId), 
		m_sliderGenerator(sliderGen),
		m_evalParams(params),
		m_hashTable(hashTable),
		m_pos{nullptr},
		m_eval{nullptr},
		m_limits{nullptr},
		m_stop{stop}
	{
		if (m_sliderGenerator == nullptr)
			throw e_searcherError(FORMAT_EXCEPTION_MESSAGE("Parameter 'sliderGen' was nullptr"));
		if (m_evalParams == nullptr)
			throw e_searcherError(FORMAT_EXCEPTION_MESSAGE("Parameter 'params' was nullptr"));
		if (m_hashTable == nullptr)
			throw e_searcherError(FORMAT_EXCEPTION_MESSAGE("Parameter 'hashTable' was nullptr"));
		m_stats = std::make_shared<search_stats>();
	}

	/// <summary>
	/// Main search function. Goes into iterative deepening
	/// </summary>
	void searcher::search(
		const position::game_state& state,
		std::shared_ptr<const search_limits>& limits)
	{
		// Prepare the object for a new search
		preprocess_search(state, limits);

		// Start the search
		auto best_move = MOVE_NULL;
		std::stringstream ss;
		for (auto current_depth = (eDepth)1; current_depth <= m_limits->depth; current_depth++)
		{
			// Clear the previous search's selective depth.
			m_stats->info.selective_depth = ZERO_DEPTH;

			auto score = root_search(depth_for_thread(current_depth), -VALUE_INF, VALUE_INF);

			if (m_stop->load(std::memory_order_relaxed))
			{
				if (current_depth == 1) /* If this is the first iteration we need to fetch the best move here. */
					best_move = m_stats->pvTable->get_for_depth(m_pos->ply()).second[0];
				break;
			}

			if (m_threadId == MAIN_THREAD)
			{
				auto nodes = get_nodes();
				auto time_to_depth = now() - m_limits->start_time;
				if (time_to_depth <= 0)
					time_to_depth = 1;
				auto nodes_per_sec = static_cast<int64_t>((double)nodes / (time_to_depth / 1000.0));
				auto pv = m_stats->pvTable->get_for_depth(m_pos->ply());
				best_move = pv.second[0];

				// Print info.
				ss.str("");
				ss.clear();
				ss << "info score ";
				if (std::abs(score) > VALUE_MATE)
					ss << "mate " << to_mate(score);
				else
					ss << "cp " << to_centipawns(score);
				ss << " depth " << current_depth
					<< " seldepth " << m_stats->info.selective_depth
					<< " nodes " << nodes
					<< " nps " << nodes_per_sec
					<< " time " << time_to_depth;
				if (pv.first > 0)
				{
					ss << " pv ";
					for (auto i = 0; i < pv.first; i++)
						ss << movegen::to_string(pv.second[i]) << " ";
				}

				std::cout << ss.str() << std::endl;
			}
		}
		if (m_threadId == MAIN_THREAD)
		{
			std::cout << "bestmove " << movegen::to_string(best_move) << std::endl;
			m_stop->store(true, std::memory_order_relaxed);
		}
	}

	/// <summary>
	/// Clears data collected during previous searches.
	/// </summary>
	void searcher::preprocess_search(
		const position::game_state& state,
		std::shared_ptr<const search_limits>& limits)
	{
		m_limits = limits;
		m_pos = position::position::create_position(
			std::make_shared<position::game_state>(state),
			m_sliderGenerator);
		m_eval = std::make_unique<evaluation::evaluator>(m_pos, m_evalParams);
		m_stats->clear();

		if (m_threadId == MAIN_THREAD)
			m_stop->store(false, std::memory_order_relaxed);
	}

	/// <summary>
	/// Generate quiet moves for quiescence, but get all if we're in check.
	/// </summary>
	const move_list_t& searcher::generate_quiescence_moves()
	{
		if (m_pos->in_check())
			return m_pos->generate_moves<movegen::ALL>();
		return m_pos->generate_moves<movegen::ACTIVE>();
	}

	/// <summary>
	/// Check if we should stop the search.
	/// Only the main thread will query stdin, whereas the other threads will check for time and whether or not main has requested a stop.
	/// </summary>
	void searcher::check_stopped_search()
	{
		if (m_threadId == MAIN_THREAD)
		{
			bool quit = false, stop = false;
			auto time_up = m_limits->use_time_management() && now() >= m_limits->end_time();
			utility::ReadInput(stop, quit);

			if (stop || quit || time_up)
				m_stop->store(true, std::memory_order_relaxed);

			if (quit)
				throw uci::engine_manager::e_quitException();
		}
	}

	/// <summary>
	/// Calculate a good search depth based on the thread's ID and the depth specified in the UCI limits.
	/// </summary>
	eDepth searcher::depth_for_thread(eDepth d) const
	{
		if (m_threadId % 2 != 0 && m_threadId != MAIN_THREAD) // Uneven thread-id's get changed depth
			return static_cast<eDepth>(d + (m_threadId / 2) + 1);
		return d;
	}
}