/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "time.h"



/// <summary>
/// Default constructor for the TimeManager class.
/// </summary>
/// <param name="use_time">A flag to signal if we even need to use time management.</param>
/// <param name="time_left">The total amount of time left.</param>
/// <param name="inc">Increment.</param>
/// <param name="movestogo">The amount of moves until next time control (default: 30).</param>
/// <param name="movetime">If we've been given an exact amount of time to search, use that.</param>
TimeManager::TimeManager(bool use_time, int time_left, int inc, int movetime, int movestogo) {
	// Step 1. Set up the start time.
	start_time = getTimeMs();

	// Step 2. Calculate the default time to search. This is just the total time divided by the amount of moves before next time control.
	movetime_set = false;

	if (use_time) {
		time_limit = true;

		
		search_time = (time_left / movestogo) + inc - TIME_BUFFER;
		total_time = time_left + inc - TIME_BUFFER;

		// Step 2A. Trick to make sure we stop early if our remaining time is under 50ms:
		if (search_time < 0) { search_time = 5; }
	}
	else {
		search_time = total_time = start_time = 0;
		time_limit = false;
	}

	// Step 2B. If we have been told a movetime, just search for this exact amount.
	if (movetime > 0) {
		movetime_set = time_limit = true;
		search_time = movetime;
	}
}