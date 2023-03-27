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


namespace loki::search
{
	main_thread::main_thread(const movegen::magics::slider_generator_t	slider_generator, evaluation::evaluation_params_t& eval_params)
		: search_thread(slider_generator, eval_params, std::make_shared<std::atomic_bool>(true)),
		m_workerThreads{}
	{}

	
	void main_thread::search(
		const position::game_state& state,
		std::shared_ptr<const search_limits>& limits)
	{
		// Prepare the object for a new search.
		m_stop_searching->store(false);
		preprocess_search(state, limits);

		// Search!
		auto best_move = MOVE_NULL;
		
		// Iterative deepening
		for (auto curr_depth = (eDepth)1; curr_depth <= static_cast<eDepth>(m_limits->depth); curr_depth++)
		{
			// Clear single-search specific data
			m_info.selective_depth = (eDepth)0;

			auto score = root_search(curr_depth, -VALUE_INF, VALUE_INF);

			if (m_stop)
			{
				// if this is the first iteration, we (obviously) don't have the PV from the previous, so we need to set it here
				// in order to not return MOVE_NULL
				if (curr_depth == 1)
					best_move = m_pvTable.get_for_depth((eDepth)m_pos->ply()).second[0];

				break;
			}

			// Collect some data about the current iteration.
			long long nodes = m_info.nodes;
			long long ttd = now() - m_limits->start_time; // "time-to-depth"
			long long nps = static_cast<int64_t>((double)nodes / ((ttd <= 0 ? 1 : ttd) / 1000.0)); // "nodes-per-second"
			auto pv = m_pvTable.get_for_depth((eDepth)m_pos->ply());
			
			// Extract best move.
			best_move = pv.second[0];

			// Print info string.
			std::stringstream ss;
			ss << "info score ";
			if (std::abs(score) > VALUE_MATE)
				ss << "mate " << to_mate(score);
			else
				ss << "cp " << to_centipawns(score);

			ss << " depth " << curr_depth
				<< " seldepth " << m_info.selective_depth
				<< " nodes " << nodes
				<< " nps " << nps
				<< " time " << ttd;
			
			ss << " pv ";
			for (auto i = 0; i < pv.first; i++)
				ss << movegen::to_string(pv.second[i]) << " ";

			std::cout << ss.str() << std::endl;
			//std::cout << "\n";
		}

		// Output best move and return
		std::cout << "bestmove " << movegen::to_string(best_move) << std::endl;
	}

	/// <summary>
	/// Check if we have reached our time limit, or if a command has been given to stop the search prematurely
	/// </summary>
	void main_thread::check_stopped_search()
	{
		if (m_limits->use_time_management(m_pos->side_to_move()) && now() >= m_limits->end_time(m_pos->side_to_move()))
			m_stop_searching->store(true);
		bool quit = false;
		utility::ReadInput(m_stop, quit);

		// If we're told to stop or quit, notify the other threads.
		if (m_stop || quit)
			m_stop_searching->store(true);
		search_thread::check_stopped_search();

		// If we're specifically told to quit, throw an exception.
		if (quit)
			throw uci::engine_manager::e_quitException();
	}

}