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


inline int find_smallest(const std::vector<double>& v) {
	double smallest_value = std::numeric_limits<double>::max();
	int smallest_index = 0;

	for (int i = 0; i < v.size(); i++) {
		if (v[i] < smallest_value) {
			smallest_value = v[i];
			smallest_index = i;
		}
	}

	return smallest_index;
}


void Neural::pick_best(std::vector<int>& index_vector, std::vector<double> losses) {
	// Step 1. Clear the index vector
	index_vector.clear();

	for (int i = 0; i < POPULATION_SIZE / 10; i++) {
		// Find the smallest element and insert its index.
		int smallest_index = find_smallest(losses);
		index_vector.push_back(smallest_index);

		// Remove the smallest element in losses.
		losses.erase(losses.begin() + smallest_index);
	}
}



void Neural::NeuralNet::train_model(int iterations) {
	assert(iterations > 0);
	
	// Step 1. Generate the training set
	Training::TrainingSet positions;
	Training::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd", positions);

	// Step 2. Copy pointers to all weights and biases.
	std::vector<int32_t*> parameters;
	copy_weights_and_biases(parameters);

	// Step 3. Generate the population with random values
	std::array<std::vector<int32_t>, POPULATION_SIZE> population;
	std::vector<double> loss;

	for (int i = 0; i < POPULATION_SIZE; i++) {
		std::vector<int32_t> individual;

		for (int i = 0; i < parameters.size(); i++) {
			individual.push_back(std::rand());
		}

		population[i] = individual;
	}

	uint64_t total_children = 0;
	uint64_t total_mutations = 0;

	// Step 4. Loop through generations
	for (int n = 0; n < iterations; n++) {

		// Step 4A. Compute the loss of each individual with a batch of positions
		loss.clear();
		int start_position = random_num(0, positions.size() - BATCH_SIZE - 1);

		for (int i = 0; i < POPULATION_SIZE; i++) {

			// Step 4A.1. Change the weights and biases in the network
			change_parameters(population[i], parameters);

			// Step 4A.2. Train the current individual
			for (int p = start_position; p < start_position + BATCH_SIZE; p++) {

				load_position(positions[p].pieceBoards);

				int32_t eval = evaluate();

				// Backpropagation
				back_propagate(positions[p].value);
				update_weights();
			}
			
			// Step 4A.3. Copy the new parameter values into the individual and compute it's error
			for (int j = 0; j < parameters.size(); j++) {
				population[i][j] = *parameters[j];
			}

			loss.push_back(compute_error(start_position, population[i], parameters, positions));
		}

		// Step 4B. Selection. Pick out the 10% of the population with the lowest loss
		std::vector<int> fittest_individuals;
		pick_best(fittest_individuals, loss);

		// Step 4C. Further selection. From the top 10%, pick two parents randomly and let them crossover
		//int parent_1 = random_num(0, fittest_individuals.size() - 1);
		//int parent_2 = 0;
		//if (parent_1 > fittest_individuals.size() / 2) {
		//	parent_2 = random_num(0, parent_1 - 1);
		//}
		//else {
		//	parent_2 = random_num(parent_1 + 1, fittest_individuals.size() - 1);
		//}
		int parent_1 = 0, parent_2 = random_num(1, fittest_individuals.size() - 1);
		assert(parent_1 != parent_2);
		
		// Step 4D. Crossover. Here we use the BLX-alpha algorithm
		int parent_1_index = fittest_individuals[parent_1];
		int parent_2_index = fittest_individuals[parent_2];

		std::vector<int32_t> p1_params = population[parent_1_index];
		std::vector<int32_t> p2_params = population[parent_2_index];

		for (int i = 0; i < POPULATION_SIZE; i++) {

			population[i].clear();
			total_children++;

			// Copy the two parents
			if (i == 0) {
				population[i] = p1_params;
			}
			else if (i == 1) {
				population[i] = p2_params;
			}
			else {
				for (int p = 0; p < parameters.size(); p++) {
					int max = (p1_params[p] > p2_params[p]) ? p1_params[p] : p2_params[p];
					int min = (max == p1_params[p]) ? p2_params[p] : p1_params[p];

					// Now we use the BLX-alpha algorithm to find the new weight for the child
					population[i].push_back(random_num(min - ALPHA * (max - min), max + ALPHA * (max - min)));
				}
			}
			// Determine if a random gene in the offspring should be mutated.
			bool mutate = ((double(std::rand()) / double(RAND_MAX)) <= MUTATION_RATE) ? true : false;
			mutate = false;
			if (mutate) { // If our gene is to mutate, we need to add or subtract a number in the alpha range
				total_mutations++;
				int p = random_num(0, parameters.size() - 1);

				int max = (p1_params[p] > p2_params[p]) ? p1_params[p] : p2_params[p];
				int min = (max == p1_params[p]) ? p2_params[p] : p1_params[p];

				//population[i][p] += (randemacher() * random_num(0, ALPHA * (max - min)));
				//population[i][p] += (randemacher() * random_num(1, 2));
				population[i][p] *= -1;
			}
		}

		std::cout << "Generation " << n << " lowest error: " << loss[fittest_individuals[0]] 
			<< ", mutation rate: " << (double(total_mutations) / double(total_children))*100.0 << "%" << std::endl;

	}



	load_position(positions[0].pieceBoards);

	int32_t eval = evaluate();

	std::cout << "Network vs HCE: " << eval << " <---> " << positions[0].value << std::endl;

}


double Neural::NeuralNet::compute_error(int start, std::vector<int32_t>& new_values, std::vector<int32_t*>& parameters, Training::TrainingSet& set) {
	// Step 1. Change the parameters
	change_parameters(new_values, parameters);

	// Step 2. Determine the batch.
	assert(set.size() > BATCH_SIZE);


	// Step 3. Now loop through the positions, load them and evaluate them
	volatile double average_error = 0.0;

	for (volatile int p = start; p < start + BATCH_SIZE; p++) {
		// Load it to the inputs
		load_position(set[p].pieceBoards);

		// Score it
		volatile int32_t eval = evaluate();
		// Now map it to a win probability and take the mean squared error.
		//average_error += std::pow(sigmoid(set[p].value) - sigmoid(eval), 2.0);
		average_error += std::pow(set[p].value - eval, 2.0);
	}
	
	// Step 4. Divide the error by the batch size and return it.
	return average_error / BATCH_SIZE;
}


void Neural::NeuralNet::change_parameters(std::vector<int32_t>& new_values, std::vector<int32_t*>& parameters) {
	assert(new_values.size() == parameters.size());

	for (int p = 0; p < parameters.size(); p++) {
		*(parameters[p]) = new_values[p];
	}
}


void Neural::NeuralNet::back_propagate(int32_t expected_output) {

	Layer* current = &layers[layers.size() - 1];
	Layer* next = nullptr;

	// Step 1. Compute the output delta.
	current->deltas[0] = 2.0 * (current->neurons[0] - expected_output);

	// Step 2. Now go back in the network
	for (int l = layers.size() - 2; l > 0; l--) {

		current = &layers[l];
		next = &layers[l + 1];

		for (int j = 0; j < next->neuron_count; j++) {
			for (int i = 0; i < current->neuron_count; i++) {
				current->deltas[i] += current->weights[j][i] * next->deltas[j];
			}
		}

		for (int i = 0; i < current->neuron_count; i++) {
			current->deltas[i] *= (current->neurons[i] > 0) ? 1.0 : 0.0;
		}
	}

}


void Neural::NeuralNet::update_weights() {

	// Step 1. Set the biases at the input layer and output layer to zero
	for (int i = 0; i < layers[0].neuron_count; i++) {
		layers[0].biases[i] = 0;
	}
	layers[layers.size() - 1].biases[0] = 0;

	Layer* current = nullptr;
	Layer* previous = nullptr;

	for (int l = 1; l < layers.size(); l++) {
		double gradient = 0;
		
		current = &layers[l];
		previous = &layers[l - 1];

		if (l != 1) {
			for (int i = 0; i < previous->neuron_count; i++) {
				previous->biases[i] -= LEARNING_RATE * previous->deltas[i];
			}
		}

		for (int j = 0; j < current->neuron_count; j++) {
			for (int i = 0; i < previous->neuron_count; i++) {
				previous->weights[j][i] -= LEARNING_RATE * previous->neurons[i] * current->deltas[j];
			}
		}

	}
}


void Neural::NeuralNet::clear_deltas() {
	for (int l = 0; l < layers.size(); l++) {
		for (int i = 0; i < layers[l].neuron_count; i++) {
			layers[l].deltas[i] = 0.0;
		}
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

	deltas = new double[neuron_count];
	memset(deltas, 0, sizeof(double) * neuron_count);

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
	delete[] deltas;
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

	deltas = new double[l.neuron_count];
	memcpy(deltas, l.deltas, sizeof(double) * l.neuron_count);

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
			pieces[pce] = pos->pieceBBS[pce][WHITE];
		}
		for (int pce = PAWN + 1; pce <= KING + 1; pce++) {

			pieces[KING + pce] = pos->pieceBBS[pce][BLACK];
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