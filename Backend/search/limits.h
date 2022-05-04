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
#ifndef LIMITS_H
#define LIMITS_H

namespace loki::search {

	class limits {
		using time_point = std::chrono::duration<std::chrono::milliseconds>;
	private:
		// All constant values are private.

		time_point			m_start_time;		/* The time the search started */
		time_point			m_time[SIDE_NB];	/* The time white and black has left. */
		DEPTH				m_max_depth;		/* The maximum depth to search to. */
		size_t				m_moves_togo;		/* The number of moves until next time control. */
		bool				m_should_quit;		/* Flag signalling Loki to quit. */
		bool				m_should_stop;		/* Flag signalling Loki to stop searching. */
		std::vector<move_t> m_moves;			/* The list of moves we're asked to search. */
	public:
		inline bool has_time_limit() const noexcept;
		inline bool time_exceeded() const noexcept;

		// Values that the search function can alter.
		DEPTH	selective_depth;				/* The max depth we've reached due to extensions/reductions. */
		size_t	nodes_searched;					/* The amount of nodes we've encountered during the search. */
	};
}

#endif