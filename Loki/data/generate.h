#ifndef GENERATE_H
#define GENERATE_H

#include "../search.h"
#include "../lnn/network.h"
#include "../lnn/trainer/data.h"


namespace DataGeneration {
    // The number of threads to use.
    constexpr int THREADS = 8;
    static_assert(THREADS >= 1);

    // The maximal batch size to use for writing incrementally.
    constexpr size_t MAX_BATCH_SIZE = 5000000;
    static_assert(MAX_BATCH_SIZE > 0);


}



#endif