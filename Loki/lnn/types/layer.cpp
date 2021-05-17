#include "layer.h"



/*
Constructor and destructor for the layer class
*/
template<size_t SIZE, size_t NEXT_SIZE, typename T>
Layer<SIZE, NEXT_SIZE, T>::Layer(){
    if constexpr (NEXT_SIZE > 0){
        weights = new T*[NEXT_SIZE];

        for (int i = 0; i < NEXT_SIZE; i++){
            weights[i] = new T[SIZE];

            // Pre-initialize the weights to zero
            memset(weights[i], 0, sizeof(T)*SIZE);
        }
    }
    else{
        weights = nullptr;
    }
}

template<size_t SIZE, size_t NEXT_SIZE, typename T>
Layer<SIZE, NEXT_SIZE, T>::~Layer(){
    if (weights != nullptr){
        for (int i = 0; i < NEXT_SIZE;i++){
            delete[] weights[i];
        }
        delete[] weights;
    }
}


// Return the size of the layer
template<size_t SIZE, size_t NEXT_SIZE, typename T>
size_t Layer<SIZE, NEXT_SIZE, T>::size() const {
    return SIZE;
}