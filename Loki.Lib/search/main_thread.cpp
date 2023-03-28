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
	main_thread::main_thread(const movegen::magics::slider_generator_t& sliderGen,
		const evaluation::evaluation_params_t& params) : searcher(MAIN_THREAD, sliderGen, params)
	{}


	void main_thread::do_search(const position::game_state& state, std::shared_ptr<const search_limits>& limits)
	{
		search(state, limits);
	}

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