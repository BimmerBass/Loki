#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "types/layer.h"
#include "types/architecture.h"

namespace LNN {

    enum LNN_FileType :int { BIN = 0, CSV = 1 };
    constexpr int OUTPUT_BOUND = 20000;

    // This structure implementation is taken from Halogen's code.
    struct Update {
        struct UpdatePoint {
            size_t index;
            int delta;
        };

        size_t size;
        std::array<UpdatePoint, 4> deltas;
    };

    /*
    Network class
    */
    class Network {
    public:
        // The function used when evaluating the position
        int evaluate(bool fast = true);

        // The function for updating the inputs incrementally.
        void do_incremental(Update& update);
        void undo_incremental();

        // Method for loading a position which will then be updated incrementally afterwards
        void load_position(std::array<int8_t, INPUT_SIZE>& pos);

        // Method for loading a network from a file. This should always be used at startup.
        template<LNN_FileType T>
        void load_net(std::string file_path);

        std::array<neuron_t, INPUT_SIZE> get_input() {
            return INPUT_LAYER.neurons;
        }

    private:
        // For the network, we use the following architecture:
        //  768 input neurons --> There are 12 pieces with 64 squares each
        //  256 neurons in the first hidden layer
        //  32 neurons in the next hidden layer
        //  32 neurons in the last hidden layer
        //  1 output neuron since this is a value-based neural network
        Layer<INPUT_SIZE, FIRST_HIDDEN_SIZE, neuron_t> INPUT_LAYER;
        Layer<FIRST_HIDDEN_SIZE, HIDDEN_STD_SIZE, neuron_t> FIRST_HIDDEN;
        Layer<HIDDEN_STD_SIZE, HIDDEN_STD_SIZE, neuron_t> SECOND_HIDDEN;
        Layer<HIDDEN_STD_SIZE, OUTPUT_SIZE, neuron_t> THIRD_HIDDEN;
        Layer<OUTPUT_SIZE, 0, neuron_t> OUTPUT_LAYER;

        // This vector holds all Update's that has been used incrementally to get to this position.
        // They are used to undo an incremental update
        std::vector<Update> changes;
    };

    constexpr int piece_conversion[2][6] = {
        {6, 7, 8, 9, 10, 11},
        {0, 1, 2, 3, 4, 5}
    };

    // piece_type should go from 0 to 5, and square should go from 0 to 63.
    inline int calculate_input_index(int piece_type, bool white, int sq) {
        return 64 * piece_conversion[(white) ? 1 : 0][piece_type] + sq;
    }
}


#endif