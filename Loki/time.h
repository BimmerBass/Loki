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
#ifndef TIME_H
#define TIME_H
#include "position.h"
#include <chrono>

/// <summary>
/// Calculate the amount of time since epoch (1970-something-something).
/// </summary>
/// <returns>The amount of milliseconds that has passed since epoch</returns>
inline long long getTimeMs() {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}


class TimeManager {
	static constexpr int TIME_BUFFER = 50;
public:
	TimeManager(bool use_time, int total, int inc, int movetime, int movestogo = 30);
	TimeManager(const TimeManager& _Rhs);


	bool time_limit_exceeded() const;
	void adjust_time(const int score_drop);
private:
	int total_time, search_time, start_time;
	bool time_limit, movetime_set;
};











#endif