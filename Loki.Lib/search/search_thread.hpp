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
#pragma once

namespace loki::search
{

	class main_thread : public searcher
	{
		EXCEPTION_CLASS(e_mainThreadError, e_searcherError);
	public:
		main_thread(const movegen::magics::slider_generator_t& sliderGen,
			const evaluation::evaluation_params_t& params);

		void do_search(const position::game_state& state, std::shared_ptr<const search_limits>& limits);

	private:
		void check_stopped_search() override;
	};
}