#ifndef LAYER_H
#define LAYER_H
#include <array>


template<size_t SIZE, size_t NEXT_SIZE, typename T>
class Layer{
    static_assert(SIZE > 0);
    static_assert(NEXT_SIZE >= 0);

    public:
        Layer();

        std::array<T,SIZE> neurons;
        std::array<T,SIZE> biases;

        // They will be declared to be indexed by weights[next_layer_neuron][this_layer_neuron]
        std::array<std::array<T, SIZE>, NEXT_SIZE> weights;

        size_t size() const;
};


#endif