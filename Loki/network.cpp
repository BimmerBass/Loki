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



/*

Below is the training implementation for the neural networks

*/


void Neural::Network::train_model(std::string epd_file, std::string net_file = "", int iterations = 1000) {

	// Step 1. If a network file has been given, load it.
	if (net_file != "") {
		load_net(net_file);
	}

	// Step 2. Copy the pointers to all weights and biases and set up all other constants/arrays
	WeightBiasVector parameters;
	copy_weight_bias_pointers(parameters);

	// Step 2A. Set up the theta_plus, theta_minus, theta and delta vector. Copy all parameter values to theta
	ThetaVector theta, theta_plus, theta_minus, delta;

	for (int i = 0; i < parameters.size(); i++) {
		theta[i] = *parameters[i];
	}

	// Step 3. Load all the positions and compute K.
	TrainingSet positions;
	load_epds(positions, epd_file);

	// Step 4. Set up the Adam vectors, zero-initialize them, and set up the SPSA constants.
	std::vector<double> adam_v, adam_m;

	for (int i = 0; i < parameters.size(); i++) {
		adam_v.push_back(0.0);
		adam_m.push_back(0.0);
	}

	double c = C_END * std::pow(double(iterations), gamma);
	double BIG_A = 0.1 * double(iterations);
	double a_end = R_END * std::pow(c, 2.0);
	double a = a_end * std::pow(BIG_A + double(iterations), alpha);

	// Step 5. Run the algorithm
	for (int n = 0; n < iterations; n++) {

		// Step 5A. Calculate all the SPSA parameters
		double cn = c / std::pow(double(n) + 1.0, gamma);
		double an = a / std::pow(BIG_A + double(n) + 1.0, alpha);

		// Step 5B. Determine the delta array and calculate theta_plus and theta_minus from that.
		for (int i = 0; i < parameters.size(); i++) {
			double d = randemacher();

			delta[i] = d;

			theta_plus[i] = std::round(theta[i] + cn * d);
			theta_minus[i] = std::round(theta[i] - cn * d);
		}

		// Step 5C. Compute the error of theta_plus and theta_minus
		double theta_plus_error = mean_square_error(theta_plus, parameters, positions);
		double theta_minus_error = mean_square_error(theta_minus, parameters, positions);

		// Step 5D. Now compute the gradient and update all weights in theta
		for (int i = 0; i < parameters.size(); i++) {

			double gradient = (theta_plus_error - theta_minus_error) / (2.0 * cn * delta[i]);

			// Now calculate the adam parameters
			adam_m[i] = BETA_ONE * adam_m[i] + (1.0 - BETA_ONE) * gradient;
			adam_v[i] = BETA_TWO * adam_v[i] + (1.0 - BETA_TWO) * std::pow(gradient, 2.0);

			double m_hat = adam_m[i] / (1.0 - std::pow(BETA_ONE, double(n) + 1.0));
			double v_hat = adam_v[i] / (1.0 - std::pow(BETA_TWO, double(n) + 1.0));

			// Update the parameter value.
			theta[i] -= (LRATE / (std::sqrt(v_hat) + EPSILON)) * m_hat;
		}

		// Step 5E. We're now done with the iteration. Compute the error of theta and display it every ten iterations
		if (n % 10 == 0) {
			double curr_error = mean_square_error(theta, parameters, positions);
			
			std::cout << "Iteration " << (n + 1) << " error: " << curr_error << std::endl;
		}
	}

	// Step 6. Now that we have a vector of optimized values (theta), insert them in the current net and save it.
	for (int i = 0; i < parameters.size(); i++) {
		*parameters[i] = theta[i];
	}

	save_net();

}



/*

The load_epds method will set up the traning set. It will save a board representation that the network should take as input, then search the position to depth 6
	and save the resulting score in centipawns

*/

void Neural::Network::load_epds(TrainingSet& s, std::string epd_file) {
	
	// Step 1. Set up some parameters.
	std::string epd = "";
	std::string fen = "";
	std::vector<std::string> FENS;
	s.clear();
	
	// Step 2. Load the epd file
	std::ifstream file(epd_file);

	// Step 2A. Load epds and extract the fen's until EOF
	while (std::getline(file, epd)) {
		fen = "";

		auto fen_end = epd.find_first_of("-");

		if (epd[fen_end + 2] == '-') {
			fen_end = fen_end + 2;
		}

		fen = epd.substr(0, fen_end + 1);

		FENS.push_back(fen);
	}

	// Step 3. Use a gamestate object to parse the fen's and then copy their network input arrays into the trainingset vector
	GameState_t* pos = new GameState_t();
	NetworkInput i;
	Bitboard pceBoard = 0;
	int sq = NO_SQ;

	for (int n = 0; n < FENS.size(); n++) {
		// Step 3A. Clear the input and parse the fen
		i.fill(0);
		pos->parseFen(FENS[n]);

		// Step 3B. Loop through the position object's bitboards and copy their values to the NetworkInput. We start with the white pieces.
		for (int pce = PAWN; pce <= KING; pce++) {
			pceBoard = pos->pieceBBS[pce][WHITE];

			while (pceBoard) {
				sq = PopBit(&pceBoard); // Find the square that a piece occupies

				// Now add this to i
				i[64 * pce + sq] = 1;
			}
		}
		for (int pce = PAWN; pce <= KING; pce++) {
			pceBoard = pos->pieceBBS[pce][BLACK];

			while (pceBoard) {
				sq = PopBit(&pceBoard); // Find the square that a piece occupies

				// Now add this to i --> we need to add 64*5 to the index since that is all the white pieces before that.
				i[64*5 + 64 * pce + sq] = 1;
			}
		}

		// Step 3C. Now add this to the training set.
		s.push_back(TrainingPosition(i, 0));

		// Step 3D. Set up a SearchInfo and SearchThread and search the position to depth 6.
		SearchInfo_t info;
		info.clear();

		info.starttime = getTimeMs();
		info.timeset = false;
		info.depth = 6;
		info.depthset = true;

		SearchThread_t ss;

		*ss.pos = *pos;
		*ss.info = info;
		SearchPv line;
		// Now search the position
		int score = Search::alphabeta(&ss, info.depth, -40000, 40000, true, &line);

		// Step 3E. Now add this score to the trainingset and we can move on to the next position
		s[n].score = int16_t(score);

		if (n % 1024 == 0) {
			std::cout << "Generated " << (n + 1) << " training positions" << std::endl;
		}
	}

	file.close();
}