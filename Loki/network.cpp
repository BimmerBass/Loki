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
	return std::min(OUTPUT_BOUND, std::max(int16_t(-OUTPUT_BOUND), output.activation));
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


// Helper function for load_net
std::vector<int16_t> split_line(std::string line) {

	std::vector<std::string> number_strings;
	std::vector<int16_t> out;

	std::istringstream iss(line);

	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		std::back_inserter(number_strings));

	for (int i = 0; i < number_strings.size(); i++) {
		out.push_back(std::stoi(number_strings[i]));
	}

	return out;
}

/*

This function will load a neural network from a file with the ".lnn" (loki-neural-network) extension.

*/

void Neural::Network::load_net(std::string file_path) {
	// Step 1. Open the file.
	std::fstream file;
	file.open(file_path, std::fstream::in);

	std::string data_str = "";
	std::vector<int16_t> data;

	// Step 2. Load all biases

	// First hidden bias
	std::getline(file, data_str);
	data = split_line(data_str);

	for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
		FIRST_HIDDEN_LAYER[i].bias = data[i];
	}

	// Standard hidden layers
	for (int n = 0; n < HIDDEN_STD_COUNT; n++) {
		std::getline(file, data_str);
		data = split_line(data_str);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			HIDDEN_LAYERS[n][i].bias = data[i];
		}
	}

	// Step 3. Load all weights

	// Input to first hidden layer weights
	std::getline(file, data_str);
	data = split_line(data_str);

	for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
		for (int j = 0; j < INPUT_SIZE; j++) {
			INPUT_TO_FIRST[i][j] = data[i * INPUT_SIZE + j];
		}
	}

	// First hidden to standard hidden
	std::getline(file, data_str);
	data = split_line(data_str);
	
	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
			FIRST_TO_HIDDEN[i][j] = data[i * FIRST_HIDDEN_SIZE + j];
		}
	}

	// Hidden layers
	for (int n = 0; n < HIDDEN_STD_COUNT; n++) {
		std::getline(file, data_str);
		data = split_line(data_str);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				HIDDEN_TO_HIDDEN[n][i][j] = data[i * HIDDEN_STD_SIZE + j];
			}
		}
	}

	// Hidden to output layer
	std::getline(file, data_str);
	data = split_line(data_str);

	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		HIDDEN_TO_OUTPUT[i] = data[i];
	}
}



/*

This function will save the current weights and biases to a ".lnn" file

*/

void Neural::Network::save_net(std::string filename) {

	// Step 1. If no filename is given, we'll denote the architecture.
	if (filename == "") {
		filename = "evalnet_";

		// Step 1A. Add all layer sizes
		filename += std::to_string(INPUT_SIZE) + "x"
			+ std::to_string(FIRST_HIDDEN_SIZE);

		for (int n = 0; n < HIDDEN_STD_COUNT; n++) {
			filename += "x" + std::to_string(HIDDEN_STD_SIZE);
		}
		// Note: The output size being 1 is implicit, so it won't be written.
	}
	// Step 1B. If there are no extension, add it.
	if (filename.find(".lnn") == std::string::npos) {
		filename += ".lnn";
	}

	// Step 2. Open the file.
	std::fstream file;
	file.open(filename, std::fstream::out);

	// Step 3. Write all the biases to the file.
	// Note: The input layer doesn't use biases to these won't be written

	file << std::to_string(FIRST_HIDDEN_LAYER[0].bias);
	for (int n = 1; n < FIRST_HIDDEN_SIZE; n++) {
		file << (" " + std::to_string(FIRST_HIDDEN_LAYER[n].bias));
	}
	file << "\n";

	for (int n = 0; n < HIDDEN_STD_COUNT; n++) {

		file << std::to_string(HIDDEN_LAYERS[n][0].bias);
		for (int i = 1; i < HIDDEN_STD_SIZE; i++) {
			file << (" " + std::to_string(HIDDEN_LAYERS[n][i].bias));
		}
		file << "\n";
	}

	// Step 4. Write all the weights to the file.
	for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
		for (int j = 0; j < INPUT_SIZE; j++) {
			file << (std::to_string(INPUT_TO_FIRST[i][j]) + " ");
		}
	}
	file << "\n";

	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
			file << (std::to_string(FIRST_TO_HIDDEN[i][j]) + " ");
		}
	}
	file << "\n";

	for (int n = 0; n < HIDDEN_STD_COUNT; n++) {

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				file << (std::to_string(HIDDEN_TO_HIDDEN[n][i][j]) + " ");
			}
		}
		file << "\n";
	}

	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		file << (std::to_string(HIDDEN_TO_OUTPUT[i]) + " ");
	}

	// Step 5. Close the file.
	file.close();
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



/*

Copy constructur

*/
Neural::Network::Network(const Network& n) {
	
	// Step 1. Copy layers
	INPUT_LAYER = n.INPUT_LAYER;
	FIRST_HIDDEN_LAYER = n.FIRST_HIDDEN_LAYER;

	for (int i = 0; i < HIDDEN_STD_COUNT; i++) {
		HIDDEN_LAYERS[i] = n.HIDDEN_LAYERS[i];
	}
	output = n.output;

	// Step 2. Copy weights
	INPUT_TO_FIRST = n.INPUT_TO_FIRST;
	FIRST_TO_HIDDEN = n.FIRST_TO_HIDDEN;

	for (int i = 0; i < HIDDEN_STD_COUNT; i++) {
		HIDDEN_TO_HIDDEN[i] = n.HIDDEN_TO_HIDDEN[i];
	}
	HIDDEN_TO_OUTPUT = n.HIDDEN_TO_OUTPUT;
}



/*

The following method returns a vector of pointers to the weights and biases we're using

*/

std::vector<int16_t*> Neural::Network::get_tuning_parameters() {
	std::vector<int16_t*> weights_and_biases;

	// Step 1. Copy all biases into it.
	for (int n = 0; n < INPUT_SIZE; n++) {
		weights_and_biases.push_back(&INPUT_LAYER[n].bias);
	}
	for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
		weights_and_biases.push_back(&FIRST_HIDDEN_LAYER[n].bias);
	}
	for (int i = 0; i < HIDDEN_STD_COUNT; i++) {
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			weights_and_biases.push_back(&HIDDEN_LAYERS[i][n].bias);
		}
	}

	// Step 2. Copy all weights.
	for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
		for (int j = 0; j < INPUT_SIZE; j++) {
			weights_and_biases.push_back(&INPUT_TO_FIRST[i][j]);
		}
	}
	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
			weights_and_biases.push_back(&FIRST_TO_HIDDEN[i][j]);
		}
	}
	for (int n = 0; n < HIDDEN_STD_COUNT; n++) {
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				weights_and_biases.push_back(&HIDDEN_TO_HIDDEN[n][i][j]);
			}
		}
	}
	for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
		weights_and_biases.push_back(&HIDDEN_TO_OUTPUT[i]);
	}

	// Step 3. Return the copied pointers
	return weights_and_biases;
}