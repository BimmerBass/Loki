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
#ifndef BENCH_H
#define BENCH_H
#include "search.h"

constexpr int BENCHMARK_DEPTH = 8;

inline void setup_params(SearchInfo_t* info) {
    
    // Step 1. Clear the object.
    info->clear();

    // Step 2. Set new values
    info->starttime = getTimeMs();
    info->depth = BENCHMARK_DEPTH;

    // Step 3. Disable the isStop flag
    Search::isStop = false;
}

namespace Bench {
    // Ethereal's bench set -- found in Berskerk/bench.cpp
    extern std::vector<std::string> benchmarks;

    extern void run_benchmark();
}



#endif