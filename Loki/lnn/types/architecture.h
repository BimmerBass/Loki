#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

/*

This header file just holds the architecture/size specifications for Loki's neural network.

*/

typedef float neuron_t;
constexpr size_t INPUT_SIZE = 768; // Twelve pieces with 64 squares each
constexpr size_t FIRST_HIDDEN_SIZE = 256;
constexpr size_t HIDDEN_STD_SIZE = 32;
constexpr size_t OUTPUT_SIZE = 1;


#endif