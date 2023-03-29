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

	inline tp_t now()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}

	struct search_limits
	{
		tp_t time[SIDE_NB] = { {0} };		/* The amount of time each side has left on the clock. */
		tp_t inc[SIDE_NB] = { {0} };		/* Increment of both sides. */
		tp_t movetime = 0;					/* The max time we can use searching. */
		tp_t start_time = now();			/* The search's start time. */
		movegen::move_list_t searchmoves{};	/* An optional list of moves we're restricted to search. */
		int movestogo = 0;					/* The amount of moves until the next time control. */
		int depth = MAX_DEPTH;				/* The max depth (plies) we're allowed to search to. */
		uint64_t nodes = 0;					/* The max amount of nodes we're allowed to search. */
		bool ponder = false;				/* Whether or not we should ponder. */
		bool infinite = false;				/* Whether or not we should search infinitely (until "stop" command) */

		// Keep a buffer to subtract from total time since it takes a little time to parse the 
		// go command and set up a new search
		inline static constexpr tp_t PARSING_BUFFER = 50;
		/// <summary>
		/// Determine whether or not Loki should use time management, given these limits.
		/// </summary>
		/// <returns>true if yes, false if no.</returns>
		bool use_time_management() const
		{
			return movetime > 0 || time[m_stm] > 0 || inc[m_stm] > 0;
		}
		/// <summary>
		/// Calculate a timepoint to end the search at.
		/// This is the only time management Loki has until a proper time management scheme has been implemented.
		/// </summary>
		tp_t end_time() const
		{
			// movetime has max precedence, so if that is set, we just return that.
			if (movetime > 0)
				return start_time + movetime;

			// Estimate movestogo, if it hasn't been specified.
			auto mtg_tmp = movestogo <= 0 ? 30 : movestogo;
			auto total_time = (time[m_stm] / mtg_tmp) + inc[m_stm] - PARSING_BUFFER;

			// Sometimes our calculated time is below 50ms, which makes total_time < 0, so we'll handle this here.
			if (total_time <= 0)
				total_time = 5;
			return start_time + total_time;
		}
	private:
		friend class search_admin;
		eSide m_stm = WHITE;

		void set_stm(eSide s) { m_stm = s; }
	};

}

#endif