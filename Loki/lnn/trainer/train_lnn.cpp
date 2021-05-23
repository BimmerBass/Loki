#include "train_lnn.h"



namespace Training {

	/*
	
	Loss function. Here we use two different types:

	MSE: Mean squared error (1/n * sum((ai - yi)^2))
	AAE: Absolute average error = (1/n * sum(|ai - yi|))
	
	*/

	template<>
	double compute_loss<LOSS_F::MSE>(const std::vector<double>& ai, const std::vector<double>& yi) {
		assert(ai.size() == yi.size());
		uint64_t sum = 0;

		for (int i = 0; i < ai.size(); i++) {
			sum += std::pow(ai[i] - yi[i], 2.0);
		}

		return static_cast<double>(sum) / ai.size();
	}

	template<>
	double compute_loss<LOSS_F::AAE>(const std::vector<double>& ai, const std::vector<double>& yi) {
		assert(ai.size() == yi.size());
		uint64_t sum = 0;

		for (int i = 0; i < ai.size(); i++) {
			sum += abs(ai[i] - yi[i]);
		}

		return static_cast<double>(sum) / ai.size();
	}

	/*
	
	Constructor of the trainer class. This is responsible for setting the session's parameters and loading the dataset.
	
	*/

	Trainer::Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss) : epochs(_epochs), batch_size(_batch_size), loss_function(loss) {
		// Step 1. Load the dataset
		load_dataset(dataset);

		// Step 2. Allocate a vector for the dataset
		training_data = new std::vector<TrainingPosition>;
	}

	Trainer::~Trainer() {
		if (training_data != nullptr) {
			delete training_data;
		}
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
				tp.network_input[i] = std::stoi(string_numbers[i]);
			}
			tp.score = std::stoi(string_numbers[string_numbers.size() - 1]);

			// Step 2C. Add this to the data vector and output to the user if a certain number of data-points has been loaded
			training_data->push_back(tp);
			current++;

			if (current % 500000 == 0) {
				std::cout << "[*] Loaded " << current << " positions" << std::endl;
			}
		}

		// Step 3. Close the file, and we're done loading the data :))
		data_file.close();
	}



	/*
	
	Backpropagation. When doing forward propagation of a single dataset, we need to propagate the error backwards in the network in order to appropriately update
		each weight and bias according to it's specific contribution to the error.
	
	*/

	void Trainer::do_backprop(neuron_t expected_value) {

		// Step 1. Compute the error for the output value.
		// Note: Since the output doesn't use an activation function in LNN, we don't need to multiply any derivative of such function. This would normally be needed.
		OUTPUT_DELTA += (loss_function == LOSS_F::AAE) ? abs(static_cast<double>(OUTPUT_LAYER.neurons[0]) - static_cast<double>(expected_value)) :
			std::pow(static_cast<double>(OUTPUT_LAYER.neurons[0]) - static_cast<double>(expected_value), 2.0);

		// Step 2. Now we can calculate the deltas for the third hidden layer

	}



	/*
	
	When we compute the deltas, we calculate the sum of each individual delta from each data point. Therefore this method is used to take the average of all deltas.
	
	*/
	void Trainer::take_avg_deltas() {

		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			FIRST_HIDDEN_DELTAS[i] /= static_cast<double>(batch_size);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			SECOND_HIDDEN_DELTAS[i] /= static_cast<double>(batch_size);
			THIRD_HIDDEN_DELTAS[i] /= static_cast<double>(batch_size);
		}

		OUTPUT_DELTA /= static_cast<double>(batch_size);
	}

	/*
	
	Set all deltas to zero. This is done before each batch step.
	
	*/
	void Trainer::clear_deltas() {
		FIRST_HIDDEN_DELTAS.fill(0.0);
		SECOND_HIDDEN_DELTAS.fill(0.0);
		THIRD_HIDDEN_DELTAS.fill(0.0);
		OUTPUT_DELTA = 0.0;
	}
}