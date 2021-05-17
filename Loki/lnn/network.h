#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <cstdint>
#include <assert.h>

#include "types/layer.h"
#include "types/architecture.h"

namespace LNN {

    class Network {
    public:
        Network();

        // The function used when evaluating the position
        int evaluate();

        // The function for updating the inputs incrementally.
        void do_incremental();
        void undo_incremental();

        // Method for loading a position which will then be updated incrementally afterwards
        void load_position(std::array<int8_t, INPUT_SIZE>& pos);

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
    };

}


#endif