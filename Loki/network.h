#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

#include "bitboard.h" // For popBit

namespace Neural {

	/*
	Network architecture definitions
	*/
	constexpr int INPUT_SIZE = 12 * 64;		/* ATM piece placement is the only thing considered */
	constexpr int FIRST_HIDDEN_SIZE = 256;	/* The first hidden layer should have 256 neurons */
	constexpr int HIDDEN_STD_SIZE = 32;		/* The two layers following should have 32 */
	constexpr int HIDDEN_STD_COUNT = 2;		/* Two standard hidden layers */
	constexpr int OUTPUT_SIZE = 1;			/* The output layer should have one neuron ==> A value network*/

	// +/- bound for the output
	constexpr int16_t OUTPUT_BOUND = 30000;


	enum A_FUNC {
		A_NONE = 0,
		RELU = 1
	};

	struct Layer {
		Layer(int n_cnt, int next_layer_len, A_FUNC a_function) {
			neuron_count = n_cnt;
			neurons = new int16_t[neuron_count];

			// There are no weights from an output layer, so if next_layer_len == 0, we shouldn't have any weights
			if (next_layer_len > 0) {
				weights = new int16_t[neuron_count * next_layer_len];
				weight_count = neuron_count * next_layer_len;
			}
			else {
				weights = nullptr;
				weight_count = 0;
			}

			biases = new int16_t[neuron_count];

			activation_function = a_function;
		}
		~Layer() {
			delete[] neurons, biases;
			if (weights != nullptr) { delete[] weights; }
		}

		int16_t* neurons;

		int16_t* weights;
		int16_t* biases;

		int neuron_count;
		int weight_count;

		A_FUNC activation_function;

		// Set all neurons to this value
		void set(int val);
	};



	class NeuralNet {
	public:
		// Our constructor takes a vector with the number of hidden layers as elements and each element corresponding to the hidden layer's length.
		NeuralNet(std::vector<int> arch);

		// This is the feedforward method for the network
		int16_t evaluate();

		// This method is responsible for loading a position. This is slow and should not be used when making and un-making moves. Only initialization
		// Note: The ordering of the bitboards array should be: WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK
		void load_position(std::array<uint64_t, 12>& bitboards);


	private:
		std::vector<Layer> layers;
	};


	// Activation function
	template<A_FUNC A>
	int16_t activation_function(int16_t x);

	inline int calculate_index(int pce, int sq) {
		assert(pce >= 0 && pce <= 11);
		assert(sq >= 0 && sq <= 63);

		return (pce * 64) + sq;
	}
}






#endif