#ifndef NETWORK_H
#define NETWORK_H
#include <array>

namespace LNN{

    typedef float neuron_t;
    constexpr size_t INPUT_SIZE = 768; // Twelve pieces with 64 squares each
    constexpr size_t FIRST_HIDDEN_SIZE = 256;
    constexpr size_t HIDDEN_STD_SIZE = 32;
    constexpr int HIDDEN_STD_COUNT = 2;
    constexpr size_t OUTPUT_SIZE = 1;
    constexpr size_t LAYER_COUNT = 5;

    template<size_t SIZE, size_t NEXT_SIZE, typename T>
    class Layer{
        static_assert(SIZE > 0);
        static_assert(NEXT_SIZE >= 0);

        public:
            Layer();
            ~Layer();

            std::array<T,SIZE> neurons;
            std::array<T,SIZE> biases;

            // For this class, the next Layer's size isn't a constant expression, so we
            // have to declare the weights on heap...
            // They will be declared to be indexed by weights[next_layer_neuron][this_layer_neuron]
            T weights**;

    };


    class Network{
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
    }

}


#endif