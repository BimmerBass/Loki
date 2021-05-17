#include "layer.h"



/*
Constructor and destructor for the layer class
*/
template<size_t SIZE, size_t NEXT_SIZE, typename T>
Layer<SIZE, NEXT_SIZE, T>::Layer(){
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