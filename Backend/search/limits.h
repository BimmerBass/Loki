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

namespace loki::search
{
	using tp_t = std::chrono::milliseconds::rep;

	struct search_limits
	{
		tp_t m_time[SIDE_NB];			/* The amount of time each side has left on the clock. */
		tp_t m_inc[SIDE_NB];			/* Increment of both sides. */
		tp_t m_movetime;				/* The max time we can use searching. */
		tp_t m_start_time;				/* The search's start time. */
		movegen::move_list_t m_moves;	/* An optional list of moves we're restricted to search. */
		int m_movestogo;				/* The amount of moves until the next time control. */
		int m_depth;					/* The max depth (plies) we're allowed to search to. */
		uint64_t m_nodes;				/* The max amount of nodes we're allowed to search. */
		bool m_ponder;					/* Whether or not we should ponder. */
		bool m_infinite;				/* Whether or not we should search infinitely (until "stop" command) */
	};

}

#endif