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


int vector_dot_product(int16_t* v1, int16_t* v2, int SIZE) { // A vector dot product is just the sum of element-wise multiplication
	int out = 0;
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}

	return out;
}

// Matrix should be indexed by matrix[COLUMN][ROW] and v should be indexed by [ROW]
//void matrix_vector_dot_product(int COLS, int ROWS, int16_t** matrix, int16_t* v, int16_t* v_out) {
//
//	for (int r = 0; r < ROWS; r++) {
//
//		vector_dot_product(matrix[r], v, v_out[r], COLS);
//
//	}
//
//}

/*



New implementation below:


*/
template<>
int16_t Neural::activation_function<Neural::A_FUNC::A_NONE>(int16_t x) {
	return double(x);
}
template<>
int16_t Neural::activation_function<Neural::A_FUNC::RELU>(int16_t x) {
	return std::max(0.0, double(x));
}


template<>
double Neural::activation_function_derivative<Neural::A_FUNC::A_NONE>(int16_t x) {
	return 1.0;
}
template<>
double Neural::activation_function_derivative<Neural::A_FUNC::RELU>(int16_t x) {
	return (x > 0) ? 1.0 : 0.0;
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
	layers.push_back(Layer(INPUT_SIZE, arch[0], A_FUNC::A_NONE, true)); // Input

	for (int l = 0; l < arch.size(); l++) { // all hidden layers. These use the ReLU activation function
		Layer L(arch[l], (l == arch.size() - 1) ? 1 : arch[l + 1], A_FUNC::RELU);
		layers.push_back(L);
		//layers.push_back(Layer(arch[l], (l == arch.size() - 1) ? 1 : arch[l + 1], A_FUNC::RELU));
	}

	layers.push_back(Layer(OUTPUT_SIZE, 0, A_FUNC::A_NONE)); // Output shouldn't be bounded by an activation function

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

	// Loop through all layers, stopping at the last hidden layer
	for (int l = 0; l < layers.size() - 1; l++) {

		Layer* current = &layers[l];
		Layer* next = &layers[l + 1];
		
		int16_t weighted_sum = 0;
		// Loop through each neuron in the next layer, calculating the dot product of our weight matrix and this layer's neurons.
		for (int n = 0; n < next->neuron_count; n++) {
			weighted_sum = vector_dot_product(current->neurons, current->weights[n], current->neuron_count);

			// Add the neuron's bias and run the resulting value through the activation function.
			next->neurons[n] = weighted_sum + next->biases[n];

			next->neurons[n] = (next->activation_function == A_FUNC::A_NONE) ? activation_function<A_FUNC::A_NONE>(next->neurons[n]) :
				activation_function<A_FUNC::RELU>(next->neurons[n]);
		}

	}

	// Return the output's value, bounded between -30k and +30k
	return layers[layers.size() - 1].neurons[0];
}





void Neural::NeuralNet::train_model(int iterations) {
	assert(iterations > 0);
	
	// Step 1. Generate the training set
	Training::TrainingSet positions;
	Training::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd", positions);

	// Step 2. Loop through all iterations
	for (int i = 0; i < iterations; i++) {
	
	}

}



/*

Constructor and destructor of Layer struc

*/

Neural::Layer::Layer(int n_cnt, int next_layer_len, A_FUNC a_function, bool is_input) {
	neuron_count = n_cnt;
	neurons = new int16_t[neuron_count];
	memset(neurons, 0, sizeof(int16_t) * neuron_count);
	next_layer_length = next_layer_len;

	// There are no weights from an output layer, so if next_layer_len == 0, we shouldn't have any weights
	if (next_layer_len > 0) {
		//weights = new int16_t[neuron_count * next_layer_len];

		weights = new int16_t*[next_layer_len];

		for (int n = 0; n < next_layer_len; n++) {
			weights[n] = new int16_t[neuron_count];
			memset(weights[n], 0, sizeof(int16_t) * neuron_count);
		}


		weight_count = neuron_count * next_layer_len;
	}
	else {
		weights = nullptr;
		weight_count = 0;
	}

	biases = new int16_t[neuron_count];
	memset(biases, 0, sizeof(int16_t) * neuron_count);

	activation_function = a_function;
}

Neural::Layer::~Layer() {
	delete[] neurons, biases;

	if (weights != nullptr) {
		
		for (int n = 0; n < next_layer_length; n++) {
			delete[] weights[n];
		}
		delete[] weights;

	}
}


Neural::Layer::Layer(const Layer& l) {

	neurons = new int16_t[l.neuron_count];
	memcpy(neurons, l.neurons, sizeof(int16_t) * l.neuron_count);

	if (l.weights != nullptr) {

		weights = new int16_t*[l.next_layer_length];

		for (int i = 0; i < l.next_layer_length; i++) {
			weights[i] = new int16_t[l.neuron_count];
			memcpy(weights[i], l.weights[i], sizeof(int16_t) * l.neuron_count);
		}
	}
	else {
		weights = nullptr;
	}

	biases = new int16_t[l.neuron_count];
	memcpy(biases, l.biases, sizeof(int16_t) * l.neuron_count);

	neuron_count = l.neuron_count;
	weight_count = l.weight_count;

	next_layer_length = l.next_layer_length;

	activation_function = l.activation_function;
}




/*

Load an EPD-file, evaluate each position and save it

*/
void Neural::Training::load_epd(std::string filepath, TrainingSet& set) {
	// Step 1. Clear the training set and open the file.
	set.clear();
	std::ifstream epd_file(filepath);

	// The data we extract from each line of the EPD.
	std::string epd = "";
	std::string fen = "";
	int eval = 0;

	// We use a GameState_t object to parse the FENs and to evaluate the positions
	GameState_t* pos = new GameState_t;
	std::array<uint64_t, 12> pieces;

	// Step 2. Parse all the epd positions
	while (std::getline(epd_file, epd)) {

		// Step 2A. Extract the FEN
		fen = "";

		size_t fen_end = epd.find_first_of("-");

		if (epd.length() > fen_end + 2 && epd[fen_end + 2] == '-') {
			fen_end = fen_end + 2;
		}

		fen = epd.substr(0, fen_end + 1);

		// Step 2B. Parse the FEN, save it to the array and evaluate the position
		pos->parseFen(fen);

		for (int pce = PAWN; pce <= KING; pce++) {
			pieces[pce] = pos->pieceBBS[PAWN][WHITE];
		}
		for (int pce = PAWN + 1; pce <= KING + 1; pce++) {

			pieces[KING + pce] = pos->pieceBBS[PAWN][BLACK];
		}
		
		eval = Eval::evaluate(pos);
		
		// If it is black's turn, we need to make the score negative since all scores should be white-relative
		if (pos->side_to_move == BLACK) {
			eval *= -1;
		}

		// Step 2C. Now add these values to the training set
		set.push_back(TrainingPosition(pieces, eval));
	}
	
	// Step 3. Tell that we're done
	std::cout << "Parsed " << set.size() << " positions and evaluated them" << std::endl;

	epd_file.close();
	delete pos;
}