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
	/// <summary>
	/// search_thread simply holds all thread-local resources required for a search, as well as references to shared ones.
	/// </summary>
	class search_thread
	{
	private:
		position::position_t m_position;
		std::unique_ptr<search_limits> m_limits;
	protected:
		search_thread(
			position::game_state_t&& state,
			movegen::magics::slider_generator_t& magicsInx,
			std::unique_ptr<search_limits>&& limPtr);

		virtual eValue search();

	private:
		eValue alpha_beta_search(eDepth depth, eValue alpha, eValue beta);
	};

}