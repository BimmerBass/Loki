#include "network.h"


/*

Mathematical helper functions

*/

int32_t vector_dot_product(int32_t* v1, int32_t* v2, int SIZE) { // A vector dot product is just the sum of element-wise multiplication
	int out = 0;
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}

	return out;
}

/*



New implementation below:


*/
template<>
int32_t Neural::activation_function<Neural::A_FUNC::A_NONE>(int32_t x) {
	return double(x);
}
template<>
int32_t Neural::activation_function<Neural::A_FUNC::RELU>(int32_t x) {
	return std::max(0.0, double(x));
}


template<>
double Neural::activation_function_derivative<Neural::A_FUNC::A_NONE>(int32_t x) {
	return 1.0;
}
template<>
double Neural::activation_function_derivative<Neural::A_FUNC::RELU>(int32_t x) {
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
int32_t Neural::NeuralNet::evaluate() {

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


void Neural::NeuralNet::copy_weights_and_biases(std::vector<int32_t*>& v) {
	// Step 1. Clear the vector
	v.clear();

	// Step 2. Loop through all layers, add biases for all hidden layers only.
	for (int l = 0; l < layers.size() - 1; l++) {
		if (l != 0) {
			for (int n = 0; n < layers[l].neuron_count; n++) {
				v.push_back(&layers[l].biases[n]);
			}
		}
		for (int i = 0; i < layers[l + 1].neuron_count; i++) {
			for (int j = 0; j < layers[l].neuron_count; j++) {
				v.push_back(&layers[l].weights[i][j]);
			}
		}
	}
}




void Neural::NeuralNet::train_model(int iterations) {
	assert(iterations > 0);
	
	// Step 1. Generate the training set
	Training::TrainingSet positions;
	Training::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd", positions);

	double BIG_A = 0.1 * double(iterations);
	double C = 4.0 * std::pow(double(iterations), 0.101);

	std::vector<int32_t*> parameters;
	copy_weights_and_biases(parameters);
	
	std::vector<int32_t> theta, theta_plus, theta_minus, delta;
	std::vector<double> adam_m, adam_v;

	for (int i = 0; i < parameters.size(); i++) {
		theta.push_back(*parameters[i]);
		theta_plus.push_back(0);
		theta_minus.push_back(0);
		delta.push_back(0);

		adam_m.push_back(0.0);
		adam_v.push_back(0.0);
	}


	// Step 2. Loop through all iterations
	for (int i = 0; i < iterations; i++) {

		double cn = C / std::pow(double(i) + 1, 0.101);

		for (int p = 0; p < parameters.size(); p++) {
			int d = randemacher();
			assert(d == 1 || d == -1);

			delta[p] = static_cast<int32_t>(d);

			theta_plus[p] = theta[p] + static_cast<int32_t>(cn * d);
			theta_minus[p] = theta[p] - static_cast<int32_t>(cn * d);
		}


		double theta_plus_error = compute_error(theta_plus, parameters, positions);
		double theta_minus_error = compute_error(theta_minus, parameters, positions);

		for (int p = 0; p < parameters.size(); p++) {
			double gradient = (theta_plus_error - theta_minus_error) / (2.0 * cn * static_cast<double>(delta[p]));

			adam_m[p] = BETA_ONE * adam_m[p] + (1.0 - BETA_ONE) * gradient;
			adam_v[p] = BETA_TWO * adam_v[p] + (1.0 - BETA_TWO) * std::pow(gradient, 2.0);

			double m_hat = adam_m[p] / (1.0 - std::pow(BETA_ONE, i + 1));
			double v_hat = adam_v[p] / (1.0 - std::pow(BETA_TWO, i + 1));

			// Finally, update theta
			theta[p] -= static_cast<int32_t>((LEARNING_RATE / (std::sqrt(v_hat) + EPSILON)) * m_hat);
		}

		// Output something each 100 iterations
		if (i % 100 == 0) {
			std::cout << "Iteration " << i << ": Theta plus error " << theta_plus_error << ", theta minus error " << theta_minus_error << std::endl;
		}
	}

}


double Neural::NeuralNet::compute_error(std::vector<int32_t>& new_values, std::vector<int32_t*>& parameters, Training::TrainingSet& set) {
	// Step 1. Change the parameters
	change_parameters(new_values, parameters);

	// Step 2. Determine the batch.
	assert(set.size() > BATCH_SIZE);
	int start_position = random_num(0, set.size() - BATCH_SIZE - 1);

	// Step 3. Now loop through the positions, load them and evaluate them
	double average_error = 0.0;

	for (int p = start_position; p < BATCH_SIZE; p++) {
		// Load it to the inputs
		load_position(set[p].pieceBoards);

		// Score it
		int32_t eval = evaluate();

		// Now map it to a win probability and take the mean squared error.
		average_error += std::pow(sigmoid(set[p].value) - sigmoid(eval), 2.0);
	}
	
	// Step 4. Divide the error by the batch size and return it.
	return average_error / static_cast<double>(BATCH_SIZE);
}


void Neural::NeuralNet::change_parameters(std::vector<int32_t>& new_values, std::vector<int32_t*>& parameters) {
	assert(new_values.size() == parameters.size());

	for (int p = 0; p < parameters.size(); p++) {
		*(parameters[p]) = new_values[p];
	}
}



/*

Constructor and destructor of Layer struc

*/

Neural::Layer::Layer(int n_cnt, int next_layer_len, A_FUNC a_function, bool is_input) {
	neuron_count = n_cnt;
	neurons = new int32_t[neuron_count];
	memset(neurons, 0, sizeof(int32_t) * neuron_count);
	next_layer_length = next_layer_len;

	// There are no weights from an output layer, so if next_layer_len == 0, we shouldn't have any weights
	if (next_layer_len > 0) {
		//weights = new int16_t[neuron_count * next_layer_len];

		weights = new int32_t*[next_layer_len];

		for (int n = 0; n < next_layer_len; n++) {
			weights[n] = new int32_t[neuron_count];
			memset(weights[n], 0, sizeof(int32_t) * neuron_count);
		}


		weight_count = neuron_count * next_layer_len;
	}
	else {
		weights = nullptr;
		weight_count = 0;
	}

	biases = new int32_t[neuron_count];
	memset(biases, 0, sizeof(int32_t) * neuron_count);

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

	neurons = new int32_t[l.neuron_count];
	memcpy(neurons, l.neurons, sizeof(int32_t) * l.neuron_count);

	if (l.weights != nullptr) {

		weights = new int32_t*[l.next_layer_length];

		for (int i = 0; i < l.next_layer_length; i++) {
			weights[i] = new int32_t[l.neuron_count];
			memcpy(weights[i], l.weights[i], sizeof(int32_t) * l.neuron_count);
		}
	}
	else {
		weights = nullptr;
	}

	biases = new int32_t[l.neuron_count];
	memcpy(biases, l.biases, sizeof(int32_t) * l.neuron_count);

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
	int32_t eval = 0;

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