#ifndef GENERATE_H
#define GENERATE_H

#include "../search.h"
#include "../lnn/network.h"


namespace DataGeneration {
    // The number of threads to use.
    constexpr int THREADS = 8;
    static_assert(THREADS >= 1);

    // Struct for holding the position to give the network and the evaluation score
    struct DataPoint{
        std::array<neuron_t, INPUT_SIZE> network_input;
        neuron_t score;
    };

    struct GenerationInfo {
        bool search_scores = false;

        int search_depth = 0;
    };

    // This function will be run by a thread and generate data.
    void generate_batch(std::vector<DataPoint>& data, const std::vector<std::string>& FENS, GenerationInfo info, bool main_thread = false);
    
    // Will spin up threads and generate the training data. Writes out to a CSV file afterwards
    void generate_training_data(std::string epd_in, std::string csv_out, bool use_search=false, int depth=0);
}



#endif