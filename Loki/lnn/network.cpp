#include "network.h"


/*

Embedded LNN. This macro will embed an LNN binary into the executable.
Note: This does not work for MSVC builds, so in these, the user will need to specify a net path manually when using LNN.

This macro invocation will declare the following three variables
    const unsigned char        gEmbeddedNNUEData[];  // a pointer to the embedded data
    const unsigned char *const gEmbeddedNNUEEnd;     // a marker to the end
    const unsigned int         gEmbeddedNNUESize;    // the size of the embedded file

*/
#include "incbin/incbin.h"


// Note: For GCC builds, default_filepath is defined in the makefile since it needs to be relative to the location, that the compiler is called at.
#if  (!defined(_MSC_VER) && defined(default_filepath))
INCBIN(EmbeddedLNNchars, default_filepath);
#else
const unsigned char gEmbeddedLNNcharsData[1] = { 0x0 };
const unsigned char* const gEmbeddedLNNcharsEnd = &gEmbeddedLNNcharsData[0];
const unsigned int gEmbeddedLNNcharsSize = 1;
#endif


/*

Split a string

*/
std::vector<std::string> split_string(std::string s, char delimiter) {
	std::vector<std::string> out;

	// Remove all spaces in the string
#ifdef _MSC_VER
	s.erase(std::remove_if(s.begin(), s.end(), std::isspace), s.end());
#else
	s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
#endif

	std::string curr_string = "";

	for (int i = 0; i < s.length(); i++) {
		// If we're at the last character, add it to the current string (if it's not the delimeter) and then to the output vector
		if (i == s.length() - 1) {
			if (s[i] != delimiter) {
				curr_string += s[i];
			}

			out.push_back(curr_string);
			continue;
		}
		// If we hit the delimeter, continue
		else if (s[i] == delimiter) {
			out.push_back(curr_string);
			curr_string = "";
			continue;
		}

		curr_string += s[i];
	}

	return out;
}


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
		Zeta.fill(0);

		// Loop through the inputs
		for (int i = 0; i < INPUT_SIZE; i++) {
			// Make sure we only have allowed values in the input array
			assert(pos[i] == 0 || pos[i] == 1);

			INPUT_LAYER.neurons[i] = static_cast<neuron_t>(pos[i]);

			// Calculate Zeta
			for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
				Zeta[n] += INPUT_LAYER.neurons[i] * INPUT_LAYER.weights[n][i];
			}
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

				// Step 1B. Apply the bias and the relu activation function
				FIRST_HIDDEN.neurons[i] += FIRST_HIDDEN.biases[i];
				FIRST_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(FIRST_HIDDEN.neurons[i]);
			}
		}
		else {
			// Step 1C. If we use the incrementally updated values, apply the relu activation to the Zeta
			for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
				FIRST_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(Zeta[i] + FIRST_HIDDEN.biases[i]);
			}
		}

		// Step 1B. For each neuron in the hidden layer, apply the activation function (do_incremental doesn't do this, so we need to do this extra loop here)
		// Also, add its bias beforehand. We do this since all weight calculations are already done, regardless of the value of "fast".
		//for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
		//	FIRST_HIDDEN.neurons[i] += FIRST_HIDDEN.biases[i];
		//	FIRST_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(FIRST_HIDDEN.neurons[i]);
		//}

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

		return std::max(-OUTPUT_BOUND, std::min(OUTPUT_BOUND, static_cast<int>(OUTPUT_LAYER.neurons[0])));
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
					//FIRST_HIDDEN.neurons[n] += INPUT_LAYER.weights[n][update.deltas[i].index];
					Zeta[n] += INPUT_LAYER.weights[n][update.deltas[i].index];
				}
			}
			else {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					//FIRST_HIDDEN.neurons[n] -= INPUT_LAYER.weights[n][update.deltas[i].index];
					Zeta[n] -= INPUT_LAYER.weights[n][update.deltas[i].index];
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
					//FIRST_HIDDEN.neurons[n] -= INPUT_LAYER.weights[n][update->deltas[i].index];
					Zeta[n] -= INPUT_LAYER.weights[n][update->deltas[i].index];
				}
			}
			else {
				for (size_t n = 0; n < FIRST_HIDDEN_SIZE; n++) {
					//FIRST_HIDDEN.neurons[n] += INPUT_LAYER.weights[n][update->deltas[i].index];
					Zeta[n] += INPUT_LAYER.weights[n][update->deltas[i].index];
				}
			}
		}

		// Step 3. Now delete the last written element
		changes.pop_back();
	}


	/*
	
	Clear the network.
	
	*/
	void Network::clear_net() {
		// Step 1. Clear all layers
		INPUT_LAYER.clear();
		FIRST_HIDDEN.clear();
		SECOND_HIDDEN.clear();
		THIRD_HIDDEN.clear();
		OUTPUT_LAYER.clear();

		// Step 2. Clear Zeta and all updates that has been done.
		Zeta.fill(0);
		changes.clear();
	}


	/*
	
	Method for loading a network file. This will be loaded form a .lnn (loki-neural-network) binary file.
	
	*/
	void Network::load_net(std::string file_path) {
		// Step 1. Clear the network and open the file
		clear_net();

		FILE* bFile = nullptr;

#if (defined(_WIN32) || defined(_WIN64))
		fopen_s(&bFile, file_path.c_str(), "rb");
#else
		bFile = fopen(file_path.c_str(), "rb");
#endif

		// Step 1A. If the file isn't open, return and give an error message
		if (bFile == nullptr) {
			std::cout << "info string error failed to open the file. Please make sure the right path is specified." << std::endl;
			return;
		}

		// Step 1B. Find the end of the file and make sure there is the right amount of data.
		fseek(bFile, 0, SEEK_END);
		uint64_t pos = ftell(bFile);
		size_t num_parameters = pos / sizeof(neuron_t);

		// Step 1C. Make sure the file exists and is properly opened.
		try {
			if (bFile == nullptr) { throw("The path specified does not contain a network file compatible with Loki."); }
			if (num_parameters != PARAMETER_COUNT) { throw("The file specified does not contain the right amount of data."); }
		}
		catch (const char* msg) {
			std::cout << "Error encountered: " << msg << std::endl;
			abort();
		}

		// Step 1C. Find the start of the file
		rewind(bFile);

		// Step 2. Read the input weights
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			fread(INPUT_LAYER.weights[i].data(), sizeof(neuron_t), INPUT_SIZE, bFile);
		}

		// Step 3. Read the first hidden layer's biases and then its weights
		fread(FIRST_HIDDEN.biases.data(), sizeof(neuron_t), FIRST_HIDDEN_SIZE, bFile);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			fread(FIRST_HIDDEN.weights[i].data(), sizeof(neuron_t), FIRST_HIDDEN_SIZE, bFile);
		}

		// Step 4. Read the second hidden layer's biases and then its weights
		fread(SECOND_HIDDEN.biases.data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			fread(SECOND_HIDDEN.weights[i].data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);
		}

		// Step 5. Read the third hidden layer's biases and then its weights
		fread(THIRD_HIDDEN.biases.data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);
		fread(THIRD_HIDDEN.weights[0].data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);

		// Step 5. Close the file and set the loaded flag to signal that we have a net.
		fclose(bFile);
		loaded = true;
	}


	/*
	
	Load an embedded net. This only works for MSVC.
	
	*/
	void Network::load_embedded() {
		// Step 1. From the filesize given by incbin, we can find out how many paramters the file contains. Throw an error if this isn't the correct number.
		size_t num_parameters = gEmbeddedLNNcharsSize / sizeof(neuron_t);

		try {
			if (num_parameters != PARAMETER_COUNT) { throw("The embedded binary does not contain the right amount of data."); }
		}
		catch (const char* msg) {
			std::cout << "Error encountered while loading embedded file: " << msg << std::endl;
			exit(EXIT_FAILURE);
		}

		// Step 2. Firstly, we need to concatenate all the chars in the arrays from incbin.h to neuron_t, which we do here.
		neuron_t gEmbeddedLNNData[PARAMETER_COUNT] = { 0 };

		for (int i = 0; i < PARAMETER_COUNT; i++) {
			memcpy(gEmbeddedLNNData + i, gEmbeddedLNNcharsData + i * (sizeof(neuron_t)), sizeof(neuron_t));
		}

		// Step 3. Copy the input weights.
		size_t current = 0;

		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			memcpy(INPUT_LAYER.weights[i].data(), gEmbeddedLNNData + i * INPUT_SIZE, INPUT_SIZE * sizeof(neuron_t));
			current += INPUT_SIZE;
		}

		// Step 4. For all the hidden layers we will copy their biases first and then their weights.

		// Step 4A. First hidden
		memcpy(FIRST_HIDDEN.biases.data(), gEmbeddedLNNData + current, FIRST_HIDDEN_SIZE * sizeof(neuron_t));
		current += FIRST_HIDDEN_SIZE;

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			memcpy(FIRST_HIDDEN.weights[i].data(), gEmbeddedLNNData + current, FIRST_HIDDEN_SIZE * sizeof(neuron_t));
			current += FIRST_HIDDEN_SIZE;
		}

		// Step 4B. Second hidden
		memcpy(SECOND_HIDDEN.biases.data(), gEmbeddedLNNData + current, HIDDEN_STD_SIZE * sizeof(neuron_t));
		current += HIDDEN_STD_SIZE;

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			memcpy(SECOND_HIDDEN.weights[i].data(), gEmbeddedLNNData + current, HIDDEN_STD_SIZE * sizeof(neuron_t));
			current += HIDDEN_STD_SIZE;
		}

		// Step 4C. Third hidden
		memcpy(THIRD_HIDDEN.biases.data(), gEmbeddedLNNData + current, HIDDEN_STD_SIZE * sizeof(neuron_t));
		current += HIDDEN_STD_SIZE;
		memcpy(THIRD_HIDDEN.weights[0].data(), gEmbeddedLNNData + current, HIDDEN_STD_SIZE * sizeof(neuron_t));
		
		current += HIDDEN_STD_SIZE;
		assert(current == PARAMETER_COUNT);

		// Step 5. Set the flag signalling that we have loaded the net successfully.
		loaded = true;
	}

}