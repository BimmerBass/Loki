#ifndef BENCH_H
#define BENCH_H
#include "search.h"

constexpr int BENCHMARK_DEPTH = 10;

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