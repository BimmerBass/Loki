#include "train_lnn.h"



namespace Training {

	/*
	Constructor. Load the dataset, allocate all neccesarry objects on heap and set hyperparameters.
	*/
	Trainer::Trainer(std::string datafile, size_t _epochs, size_t _batch_size, LOSS_F _loss, size_t _threads, double eta_start, double eta_decay)
		: epochs(_epochs), batch_size(_batch_size), loss_function(_loss), thread_count(_threads), initial_learning_rate(eta_start), learning_rate_decay(eta_decay) {

		// Step 1. Make sure all hyperparameters are in their proper ranges
		try {
			if (datafile == "") { throw("Dataset should contain the path to a CSV file."); }
			if (_epochs <= 0) { throw("Epochs should be a positive number."); }
			if (_batch_size <= 0) { throw("Batch size should be a positive number."); }
			if (_threads <= 0) { throw("Threads should be a positive number."); }
			if (eta_start <= 0) { throw("Learning rate should be a positive number."); }
			if (eta_decay <= 0) { throw("Learning rate decay should be a positive number."); }
		}
		catch (const char* msg) { // If the hyperparameters aren't configured properly, abort
			std::cout << "[!] Exception thrown by Trainer::Trainer(): " << msg << std::endl;
			abort();
		}
		// Step 2. Allocate a vector for the dataset and a vector of deltas with size _threads.
		training_data = new std::vector<TrainingPosition>;

		for (size_t t = 0; t < thread_count; t++) {
			thread_data.push_back(new ThreadData);
		}
		main_thread_data = new ThreadData;

		// Step 3. Load the dataset
		load_dataset(datafile);
	}

	/*
	Destructor. De-allocate all objects from heap
	*/
	Trainer::~Trainer() {
		if (training_data != nullptr) { delete training_data; }
		for (size_t t = 0; t < thread_data.size(); t++) {
			if (thread_data[t] != nullptr) { delete thread_data[t]; }
		}
		if (main_thread_data != nullptr) { delete main_thread_data; }
	}



	/*

	Load a CSV-file dataset. This should be formatted such that for each line, the first 786 numbers are the input values and the last one is the score.

	*/
	void Trainer::load_dataset(std::string filepath) {
		assert(filepath != "");
		assert(training_data != nullptr);

		// Step 1. Open the data file.
		std::ifstream data_file(filepath);
		std::vector<std::string> string_numbers;
		std::string line = "";

		if (!data_file.is_open()) {
			std::cout << "[!] Error loading the data. Please make sure the file exists and that the right path is given" << std::endl;
			abort();
		}

		// Step 2. Now go through all lines in the file.
		size_t current = 0;
		while (std::getline(data_file, line)) {

			// Step 2A. Split the string
			string_numbers.clear();
			string_numbers = split_string(line, ';');
			assert(string_numbers.size() == INPUT_SIZE + 1);

			// Step 2B. Copy the inputs/outputs in string_numbers to a new training position object
			TrainingPosition tp;
			tp.set(0);

			for (int i = 0; i < INPUT_SIZE; i++) {
				tp.network_inputs[i] = std::stoi(string_numbers[i]);
			}
			tp.score = std::stoi(string_numbers[string_numbers.size() - 1]);

			// Step 2C. Add this to the data vector and output to the user if a certain number of data-points has been loaded
			training_data->push_back(tp);
			current++;

			if (current % 100000 == 0) {
				std::cout << "[*] Loaded " << current << " positions" << std::endl;
			}
		}

		// Step 3. Close the file, and we're done loading the data :))
		data_file.close();
	}


	/*
	
	Forward propagation. The Network class already has evaluate(), but we need to write the weighted sums of all neurons to the ThreadData objects, which means
		we need a new method.
	
	*/
	int Trainer::forward_propagate(int thread_id) {

		// Step 1. Compute the first hidden layer.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			// Step 1A. Calculate the dot product between the input neurons and their weights to neuron "i" in the first hidden layer
			dot_product<neuron_t, INPUT_SIZE>(thread_data[thread_id]->INPUT_NEURONS, INPUT_LAYER.weights[i], thread_data[thread_id]->FIRST_HIDDEN_NEURONS[i]);

			// Step 1B. Add the bias.
			thread_data[thread_id]->FIRST_HIDDEN_NEURONS[i] += FIRST_HIDDEN.biases[i];
		}

		// Step 2. Compute the second hidden layer.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			// Step 2A. Calculate the dot product of the weights into the current neuron.
			// Note: We didn't apply the activation function in the first hidden layer before, so this should be done now.
			dot_product<neuron_t, FIRST_HIDDEN_SIZE>(apply_ReLU<neuron_t, FIRST_HIDDEN_SIZE>(thread_data[thread_id]->FIRST_HIDDEN_NEURONS),
				FIRST_HIDDEN.weights[i], thread_data[thread_id]->SECOND_HIDDEN_NEURONS[i]);

			// Step 2B. Apply the bias
			thread_data[thread_id]->SECOND_HIDDEN_NEURONS[i] += SECOND_HIDDEN.biases[i];
		}

		// Step 3. Do the same for the third layer as in step 2.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, HIDDEN_STD_SIZE>(apply_ReLU<neuron_t, HIDDEN_STD_SIZE>(thread_data[thread_id]->SECOND_HIDDEN_NEURONS),
				SECOND_HIDDEN.weights[i], thread_data[thread_id]->THIRD_HIDDEN_NEURONS[i]);
			thread_data[thread_id]->THIRD_HIDDEN_NEURONS[i] += THIRD_HIDDEN.biases[i];
		}

		// Step 4. Now compute the output from the third hidden layer.
		// Note: Apply the relu and don't add a bias to the output.
		dot_product<neuron_t, HIDDEN_STD_SIZE>(apply_ReLU<neuron_t, HIDDEN_STD_SIZE>(thread_data[thread_id]->THIRD_HIDDEN_NEURONS),
			THIRD_HIDDEN.weights[0], thread_data[thread_id]->OUTPUT_NEURON);

		return std::max(-LNN::OUTPUT_BOUND, std::min(LNN::OUTPUT_BOUND, static_cast<int>(thread_data[thread_id]->OUTPUT_NEURON)));
	}
}