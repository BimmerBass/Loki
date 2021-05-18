#include "network.h"


/*

Compute the dot-product between two vectors (std::array).

*/
template<typename T, size_t SIZE>
void dot_product(std::array<T, SIZE>& v1, std::array<T, SIZE>& v2, T& out) {

	out = 0;

	// A vector dot product is just a sum of element-wise multiplications
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}
}


namespace LNN {

	/*
	
	Load a position into the input. This will be done when we're given the position by the GUI.
	Afterwards we'll use the incremental update methods for speed.

	*/
	void Network::load_position(std::array<int8_t, INPUT_SIZE>& pos) {

		// Loop through the inputs
		for (int i = 0; i < INPUT_SIZE; i++) {
			// Make sure we only have allowed values in the input array
			assert(pos[i] == 0 || pos[i] == 1);

			INPUT_LAYER.neurons[i] = static_cast<neuron_t>(pos[i]);
		}
	}


}