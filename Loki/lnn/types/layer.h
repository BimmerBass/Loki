#ifndef LAYER_H
#define LAYER_H
#include <array>


template<size_t SIZE, size_t NEXT_SIZE, typename T>
class Layer{

    public:
        Layer();

        std::array<T,SIZE> neurons;
        std::array<T,SIZE> biases;

        // They will be declared to be indexed by weights[next_layer_neuron][this_layer_neuron]
        std::array<std::array<T, SIZE>, NEXT_SIZE> weights;

        size_t size() const;

        void clear();
};



// NOTE: The following functions are defined in the header since there are linker errors if they're in their own .cpp file.
/*
Constructor and destructor for the layer class
*/
template<size_t SIZE, size_t NEXT_SIZE, typename T>
Layer<SIZE, NEXT_SIZE, T>::Layer() {
    // Initialize all neurons, weights and biases to a value of 0
    neurons.fill(0);
    biases.fill(0);
    for (int i = 0; i < NEXT_SIZE; i++) {
        weights[i].fill(0);
    }
}


// Return the size of the layer
template<size_t SIZE, size_t NEXT_SIZE, typename T>
size_t Layer<SIZE, NEXT_SIZE, T>::size() const {
    return SIZE;
}

// Clears all parameters in the layer
template<size_t SIZE, size_t NEXT_SIZE, typename T>
void Layer<SIZE, NEXT_SIZE, T>::clear() {
    neurons.fill(0);
    biases.fill(0);
    for (int i = 0; i < NEXT_SIZE; i++) {
        weights[i].fill(0);
    }
}

#endif