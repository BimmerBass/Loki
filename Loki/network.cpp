#include "network.h"


/*

Mathematical helper functions

*/

//template<size_t SIZE>
//void vector_dot_product(const std::array<int16_t, SIZE>& w, const std::array<Neural::Neuron, SIZE>& l, Neural::Neuron& out, bool ReLU, bool apply_bias) {
//	out.activation = 0;
//
//	for (int i = 0; i < SIZE; i++) {
//		out.activation += w[i] * l[i].activation;
//	}
//
//	// If apply_bias is true, add the bias
//	if (apply_bias) {
//		out.activation += out.bias;
//	}
//
//
//	// If ReLU is true, just apply the relu activation function
//	if (ReLU) {
//		out.activation = std::max(int16_t(0), out.activation);
//	}
//}
//
//template<size_t ROWS, size_t COLS>
//void matrix_vector_dot_product(const std::array<std::array<int16_t, COLS>, ROWS>& M, const std::array<Neural::Neuron, COLS>& L, std::array<Neural::Neuron, ROWS>& O) {
//
//	// Each row in the output vector is the dot product of that row number in the matrix, and the input vector
//	for (int r = 0; r < ROWS; r++) {
//		
//		vector_dot_product<COLS>(M[r], L, O[r], true, true);
//	}
//}

template<size_t SIZE>
void vector_dot_product(int16_t* v1, int16_t* v2, int16_t& out) { // A vector dot product is just the sum of element-wise multiplication
	out = 0;
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}
}

/*



New implementation below:


*/
template<>
int16_t Neural::activation_function<Neural::A_NONE>(int16_t x) {
	return double(x);
}
template<>
int16_t Neural::activation_function<Neural::RELU>(int16_t x) {
	return std::max(0.0, double(x));
}


void Neural::Layer::set(int val) {
	for (int n = 0; n < neuron_count; n++) {
		neurons[n] = val;
	}
}



Neural::NeuralNet::NeuralNet(std::vector<int> arch) {
	assert(arch.size() > 0);
	// Step 1. Allocate the hidden layers, plus the input and output
	layers.clear();
	layers.push_back(Layer(INPUT_SIZE, arch[0], A_NONE)); // Input

	for (int l = 0; l < arch.size(); l++) { // all hidden layers. These use the ReLU activation function
		layers.push_back(Layer(arch[l], (l == arch.size() - 1) ? 1 : arch[l + 1], RELU));
	}

	layers.push_back(Layer(OUTPUT_SIZE, 0, A_NONE)); // Output shouldn't be bounded by an activation function

}


// Set up all pieces such that an input neuron with value 1 designates a piece's presence on that square
void Neural::NeuralNet::load_position(std::array<uint64_t, 12>& bitboards) {
	// Step 1. Set the input layer to zero
	layers[0].set(0);
	
	// Step 2. Load the position
	uint64_t pieceBoard = 0;
	int sq = 0;

	for (int pce = 0; pce < 12; pce++) {
		pieceBoard = bitboards[pce];

		// Find all high bits.
		while (pieceBoard) {
			sq = PopBit(&pieceBoard);

			layers[0].neurons[calculate_index(pce, sq)] = 1;
		}
	}
}

// Feed forward
int16_t Neural::NeuralNet::evaluate() {

}