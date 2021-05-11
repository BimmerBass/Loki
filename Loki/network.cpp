#include "network.h"


/*

Mathematical helper functions

*/

template<size_t SIZE>
void vector_dot_product(const std::array<int16_t, SIZE>& w, const std::array<Neural::Neuron, SIZE>& l, Neural::Neuron& out, bool ReLU, bool apply_bias) {
	out.activation = 0;

	for (int i = 0; i < SIZE; i++) {
		out.activation += w[i] * l[i].activation;
	}

	// If apply_bias is true, add the bias
	if (apply_bias) {
		out.activation += out.bias;
	}


	// If ReLU is true, just apply the relu activation function
	if (ReLU) {
		out.activation = std::max(int16_t(0), out.activation);
	}
}

template<size_t ROWS, size_t COLS>
void matrix_vector_dot_product(const std::array<std::array<int16_t, COLS>, ROWS>& M, const std::array<Neural::Neuron, COLS>& L, std::array<Neural::Neuron, ROWS>& O) {

	// Each row in the output vector is the dot product of that row number in the matrix, and the input vector
	for (int r = 0; r < ROWS; r++) {
		
		vector_dot_product<COLS>(M[r], L, O[r], true, true);
	}
}

/*

After having loaded a position into the network, this method will be run to evaluate it.

*/
int16_t Neural::Network::evaluate() {
	// Since we have already calculated until the first hidden layer, we just need to calculate the rest.
	// Step 1. Calculate the first standard hidden layer
	matrix_vector_dot_product<HIDDEN_STD_SIZE, FIRST_HIDDEN_SIZE>(FIRST_TO_HIDDEN, FIRST_HIDDEN_LAYER, HIDDEN_LAYERS[0]);

	// Step 2. Calculate the next standard hidden layers
	for (int n = 1; n < HIDDEN_STD_COUNT; n++) {
		matrix_vector_dot_product<HIDDEN_STD_SIZE, HIDDEN_STD_SIZE>(HIDDEN_TO_HIDDEN[n], HIDDEN_LAYERS[n - 1], HIDDEN_LAYERS[n]);
	}

	// Step 3. Calculate the output. Here we just need to take a standard vector dot product
	// Note: We don't use an activation function for the output.
	vector_dot_product<HIDDEN_STD_SIZE>(HIDDEN_TO_OUTPUT, HIDDEN_LAYERS[HIDDEN_STD_COUNT - 1], output, false, false);

	// Step 4. Return the activation of the output, but make sure it stays inside +- 30000
	return std::min(INF, std::max(int16_t(-INF), output.activation));
}


/*

The load_position method loads a position and calculates the first hidden layer.

*/
void Neural::Network::load_position(std::array<int16_t, INPUT_SIZE>& inputs) {
	// Step 1. Copy the inputs
	for (int i = 0; i < INPUT_SIZE; i++) {
		INPUT_LAYER[i].activation = inputs[i];
		INPUT_LAYER[i].bias = 0;
	}

	// Step 2. Calculate the first hidden layer.
	matrix_vector_dot_product<FIRST_HIDDEN_SIZE, INPUT_SIZE>(INPUT_TO_FIRST, INPUT_LAYER, FIRST_HIDDEN_LAYER);
}


/*

This function will load a neural network from a file with the ".lnn" (loki-neural-network) extension.

*/

void Neural::Network::load_net(std::string file_path) {

}



/*

The constructor is used to initialize the network. If not given a path to a working network file, it will do so randomly.

*/
Neural::Network::Network(std::string net_file) {
	if (net_file != "") {
		load_net(net_file);
	}

	else {
		// Step 1. Initialize all weights randomly.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < INPUT_SIZE; j++){
				INPUT_TO_FIRST[i][j] = std::rand();
			}
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
				FIRST_TO_HIDDEN[i][j] = std::rand();
			}
		}

		for (int n = 0; n < HIDDEN_STD_COUNT; n++) {

			for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
				for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
					HIDDEN_TO_HIDDEN[n][i][j] = std::rand();
				}
			}
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			HIDDEN_TO_OUTPUT[i] = std::rand();
		}


		// Step 2. Initialize all biases randomly
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			FIRST_HIDDEN_LAYER[i].bias = std::rand();
		}

		for (int n = 0; n < HIDDEN_STD_COUNT; n++) {
			for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
				HIDDEN_LAYERS[n][i].bias = std::rand();
			}
		}

		// Output bias should always be zero.
		output.bias = 0;
	}
}