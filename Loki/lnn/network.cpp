#include "network.h"


/*

Compute the dot-product between two vectors (std::array).

*/
template<typename T, size_t SIZE>
void dot_product(const std::array<T, SIZE>& v1, const std::array<T, SIZE>& v2, T& out) {

	out = 0;

	// A vector dot product is just a sum of element-wise multiplications
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}
}

// ReLU activation function
template<typename T>
T apply_ReLU(T v) {
	return std::max(T(0), v);
}

namespace LNN {

	/*
	
	Load a position into the input. This will be done when we're given the position by the GUI.
	Afterwards we'll use the incremental update methods for speed.

	*/
	void Network::load_position(std::array<int8_t, INPUT_SIZE>& pos) {

		INPUT_LAYER.neurons.fill(0);

		// Loop through the inputs
		for (int i = 0; i < INPUT_SIZE; i++) {
			// Make sure we only have allowed values in the input array
			assert(pos[i] == 0 || pos[i] == 1);

			INPUT_LAYER.neurons[i] = static_cast<neuron_t>(pos[i]);
		}
	}


	/*
	
	Evaluate a position through forward-propagation. This is the main method used in the network class.
	The parameter "fast" is used to determine how much of the net should be calculated.
		- true: Calculate from the first hidden layer, which is already calculated itself from the inverse updates.
		- false: Calculate the entire network from the input
	*/
	int Network::evaluate(bool fast) {

		// Step 1. If we're told to do a full calculation, compute the first hidden layer
		if (!fast) {

			for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {

				// Step 1A. Calculate the dot product of the input neurons and the weights between the layers
				dot_product<neuron_t, INPUT_SIZE>(INPUT_LAYER.neurons, INPUT_LAYER.weights[i], FIRST_HIDDEN.neurons[i]);

			}
		}

		// Step 1B. For each neuron in the hidden layer, apply the activation function (do_incremental doesn't do this, so we need to do this extra loop here)
		// Also, add its bias beforehand. We do this since all weight calculations are already done, regardless of the value of "fast".
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			FIRST_HIDDEN.neurons[i] += FIRST_HIDDEN.biases[i];
			FIRST_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(FIRST_HIDDEN.neurons[i]);
		}

		// Step 2. Now calculate all other layers starting with first hidden to second hidden
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, FIRST_HIDDEN_SIZE>(FIRST_HIDDEN.neurons, FIRST_HIDDEN.weights[i], SECOND_HIDDEN.neurons[i]);

			SECOND_HIDDEN.neurons[i] += SECOND_HIDDEN.biases[i];
			SECOND_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(SECOND_HIDDEN.neurons[i]);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, HIDDEN_STD_SIZE>(SECOND_HIDDEN.neurons, SECOND_HIDDEN.weights[i], THIRD_HIDDEN.neurons[i]);

			THIRD_HIDDEN.neurons[i] += THIRD_HIDDEN.biases[i];
			THIRD_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(THIRD_HIDDEN.neurons[i]);
		}
		

		// Step 3. Now calculate the output, but don't apply an activation function or a bias
		dot_product<neuron_t, HIDDEN_STD_SIZE>(THIRD_HIDDEN.neurons, THIRD_HIDDEN.weights[0], OUTPUT_LAYER.neurons[0]);

		return static_cast<int>(OUTPUT_LAYER.neurons[0]);
	}


	/*
	Make a move incrementally
	*/
	void Network::do_incremental(Update& update) {

		for (size_t i = 0; i < update.size; i++) {

			assert((INPUT_LAYER.neurons[update.deltas[i].index] == 0 && update.deltas[i].delta == 1)
				|| (INPUT_LAYER.neurons[update.deltas[i].index] == 1 && update.deltas[i].delta == -1));

			// Step 1. Update the input.
			INPUT_LAYER.neurons[update.deltas[i].index] += static_cast<neuron_t>(update.deltas[i].delta);

			// Step 2. Now update all connections to the first hidden layer from this, updated, neuron
			if (update.deltas[i].delta == 1) {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					FIRST_HIDDEN.neurons[n] += INPUT_LAYER.weights[n][update.deltas[i].index];
				}
			}
			else {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					FIRST_HIDDEN.neurons[n] -= INPUT_LAYER.weights[n][update.deltas[i].index];
				}
			}

		}

		// Step 3. Now save this update point such that we can easily undo the incremental update later on.
		changes.push_back(update);
	}


	/*
	Undo a move. We just delete the latest change of the input layer
	*/
	void Network::undo_incremental() {
		assert(changes.size() > 0);

		// Copy the last change made in the network
		Update* update = &changes[changes.size() - 1];

		for (size_t i = 0; i < update->size; i++) {
			assert((INPUT_LAYER.neurons[update->deltas[i].index] == 0 && update->deltas[i].delta == -1)
				|| (INPUT_LAYER.neurons[update->deltas[i].index] == 1 && update->deltas[i].delta == 1));

			// Step 1. Change the input back
			INPUT_LAYER.neurons[update->deltas[i].index] -= static_cast<neuron_t>(update->deltas[i].delta);

			// Step 2. Change all connections to the first hidden layer
			if (update->deltas[i].delta == 1) {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					FIRST_HIDDEN.neurons[n] -= INPUT_LAYER.weights[n][update->deltas[i].index];
				}
			}
			else {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					FIRST_HIDDEN.neurons[n] += INPUT_LAYER.weights[n][update->deltas[i].index];
				}
			}
		}

		// Step 3. Now delete the last written element
		changes.pop_back();
	}

}