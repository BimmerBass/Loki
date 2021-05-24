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

	Trainer::Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss, double lRate, double lRate_decay)
		: epochs(_epochs), batch_size(_batch_size), loss_function(loss), initial_learning_rate(lRate), learning_rate_decay(lRate_decay) {
		
		// Step 1. Allocate a vector for the dataset
		training_data = new std::vector<TrainingPosition>;
		
		// Step 2. Load the dataset
		load_dataset(dataset);
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

			if (current % 100000 == 0) {
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

	void Trainer::do_backprop(volatile double expected_value) {

		// Step 1. Reset the changes in the deltas
		clear_delta_changes();

		// Step 2. Compute the error for the output value.
		// Note: Since the output doesn't use an activation function in LNN, we don't need to multiply any derivative of such function. This would normally be needed.
		double diff = double(OUTPUT_LAYER.neurons[0]) - expected_value;
		OUTPUT_DELTA_CHANGE = sigmoid_derivative(OUTPUT_LAYER.neurons[0]);

		if (loss_function == LOSS_F::AAE) {
			OUTPUT_DELTA_CHANGE *= (diff > 0) ? 1.0 : -1.0;
		}
		else {
			OUTPUT_DELTA_CHANGE *= 2 * (diff);
		}

		OUTPUT_DELTA += OUTPUT_DELTA_CHANGE;

		// Step 3. Now we can calculate the deltas for the third hidden layer
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			THIRD_HIDDEN_DELTAS_CHANGES[n] = static_cast<double>(THIRD_HIDDEN.weights[0][n]) * OUTPUT_DELTA_CHANGE * ReLU_derivate(THIRD_HIDDEN.neurons[n]);
			THIRD_HIDDEN_DELTAS[n] += THIRD_HIDDEN_DELTAS_CHANGES[n];
		}

		// Step 4. Calculate the second hidden layer's deltas.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) { // Each neuron in third hidden layer
			for (int n = 0; n < HIDDEN_STD_SIZE; n++) { // Each neuron in second hidden layer
				SECOND_HIDDEN_DELTAS_CHANGES[n] += static_cast<double>(SECOND_HIDDEN.weights[m][n]) * THIRD_HIDDEN_DELTAS_CHANGES[m];
			}
		}

		// Step 4A. Multiply all the deltas with the activation function derivative
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			SECOND_HIDDEN_DELTAS_CHANGES[n] *= ReLU_derivate(SECOND_HIDDEN.neurons[n]);
			SECOND_HIDDEN_DELTAS[n] += SECOND_HIDDEN_DELTAS_CHANGES[n];
		}

		// Step 5. Lastly, calculate the first hidden layer's deltas.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
			for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
				FIRST_HIDDEN_DELTAS_CHANGES[n] += static_cast<double>(FIRST_HIDDEN.weights[m][n]) * SECOND_HIDDEN_DELTAS_CHANGES[m];
			}
		}

		// Step 5A. Multiply these deltas by the activation function derivatives and we're done :))
		for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
			FIRST_HIDDEN_DELTAS_CHANGES[n] *= ReLU_derivate(FIRST_HIDDEN.neurons[n]);
			FIRST_HIDDEN_DELTAS[n] += FIRST_HIDDEN_DELTAS_CHANGES[n];
		}

	}


	/*
	
	Weight update. When we have done backpropagation on all the data points in the set (given by the batch size), update the weights depending on their
		deltas.
	
	*/
	void Trainer::update_weights() {
		volatile double weight_gradient = 0.0, bias_gradient = 0.0;

		// Step 1. Update the weights between the output and the third hidden layer.
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			weight_gradient = THIRD_HIDDEN.neurons[n] * OUTPUT_DELTA;
			bias_gradient = OUTPUT_DELTA;

			// Update the weight and bias.
			THIRD_HIDDEN.weights[0][n] -= learning_rate * weight_gradient;
			THIRD_HIDDEN.biases[n] -= learning_rate * bias_gradient;
		}

		// Step 2. Update the weights and biases of the second hidden layer.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) { // For each neuron in the third hidden layer
			for (int n = 0; n < HIDDEN_STD_SIZE; n++) { // For each neuron in the second hidden layer
				weight_gradient = SECOND_HIDDEN.neurons[n] * THIRD_HIDDEN_DELTAS[m];
				bias_gradient = THIRD_HIDDEN_DELTAS[m];

				// Update the weight and bias
				SECOND_HIDDEN.weights[m][n] -= learning_rate * weight_gradient;
				SECOND_HIDDEN.biases[n] -= learning_rate * bias_gradient;
			}
		}

		// Step 3. Update weights and biases for the first hidden layer
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
			for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
				weight_gradient = FIRST_HIDDEN.neurons[n] * SECOND_HIDDEN_DELTAS[m];
				bias_gradient = SECOND_HIDDEN_DELTAS[m];

				// Update the weight and the bias
				FIRST_HIDDEN.weights[m][n] -= learning_rate * weight_gradient;
				FIRST_HIDDEN.biases[n] -= learning_rate * bias_gradient;
			}
		}

		// Step 4. Lastly, update the weights from the input to the first hidden layer.
		// Note: The input shouldn't have any bias, so this won't be updated.
		for (int m = 0; m < FIRST_HIDDEN_SIZE; m++) {
			for (int n = 0; n < INPUT_SIZE; n++) {
				weight_gradient = INPUT_LAYER.neurons[n] * FIRST_HIDDEN_DELTAS[m];
				
				// Update the weight
				INPUT_LAYER.weights[m][n] -= learning_rate * weight_gradient;
			}
		}


	}


	/*
	
	Parameter initialization.
		If we aren't loading a network from a file, we need to initialize it with random values. The Network class initializes all weights/biases to zero,
		which won't work with the backpropagation algorithm.
	
	*/
	inline neuron_t random_num(int start, int end) {
		
		neuron_t f = static_cast<neuron_t>(std::rand()) / static_cast<neuron_t>(RAND_MAX);
		return static_cast<neuron_t>(start) + f * (static_cast<neuron_t>(end) - static_cast<neuron_t>(start));
	}

	void Trainer::init_parameters() {
		
		// Step 1. Seed the RNG randomly
		std::srand(std::time(nullptr));
		
		// Step 2. Initialize the input randomly
		INPUT_LAYER.biases.fill(neuron_t(0));

		for (int n = 0; n < INPUT_SIZE; n++) {
			for (int m = 0; m < FIRST_HIDDEN_SIZE; m++) {
				INPUT_LAYER.weights[m][n] = random_num(-1, 1);
			}
		}

		// Step 3. Initialize weights and biases of the first hidden layer
		for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {

			FIRST_HIDDEN.biases[n] = random_num(-1, 1);

			for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
				FIRST_HIDDEN.weights[m][n] = random_num(-1, 1);
			}
		}

		// Step 4. Initialize the second hidden layer
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {

			SECOND_HIDDEN.biases[n] = random_num(-1, 1);

			for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
				SECOND_HIDDEN.weights[m][n] = random_num(-1, 1);
			}
		}

		// Step 5. Initialize the third hidden layer
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {

			THIRD_HIDDEN.biases[n] = random_num(-1, 1);

			THIRD_HIDDEN.weights[0][n] = random_num(-1, 1);
		}

		// Step 6. Set the output bias to 0
		OUTPUT_LAYER.biases[0] = neuron_t(0);
	}



	/*
	
	Train a model. This is the main method of the Trainer class and it is responsible for training a network.
	
	*/
	void Trainer::train_model(std::string saved_model) {
		assert(epochs > 0);
		assert(batch_size > 0 && batch_size < training_data->size());
		assert(learning_rate_decay > 0);

		// Step 1. If we have a saved model that we want to train further, load it. Otherwise, initialize all relevant weights and biases randomly.
		if (saved_model != "") {
			if (saved_model.find(".csv") != std::string::npos) {
				load_net<LNN::CSV>(saved_model);
			}
			else {
				load_net<LNN::BIN>(saved_model);
			}
		}
		else {
			init_parameters();
		}


		// Step 2. We can now start the training.
		std::cout << "+----------------------------------------------------------------+\n"
				  << "|                     Loki NN tuning session                     |\n"
				  << "+---------------------+------------------------------------------+\n"
				  << "| Size of dataset		| " << training_data->size() << "\n"
				  << "| Epochs				| " << epochs << "\n"
				  << "| Batch size			| " << batch_size << "\n"
				  << "| Loss function		| " << ((loss_function == LOSS_F::AAE) ? "Average absolute error" : "Mean squared error") << "\n"
				  << "| Learning rate		| " << learning_rate << "\n"
				  << "| Learning decay rate	| " << learning_rate_decay << "\n"
				  << "+---------------------+------------------------------------------+" << std::endl;

		// These are used for computing the loss
		std::vector<double> outputs;
		std::vector<double> expected;


		for (int e = 0; e < epochs; e++) {
			// Calculate this epoch's learning rate
			learning_rate = calc_learning_rate(e);
			assert(learning_rate > 0);

			// Step 2A. Sub-divide the data into batches
			std::vector<size_t> batches;
			int batch_count = training_data->size() / batch_size;
			int remainder = training_data->size() % batch_size;

			for (int i = 0; i < batch_count; i++) {
				batches.push_back(i * batch_size);
			}
			if (remainder > 0) {
				batches.push_back(remainder);
			}
			batches.push_back(training_data->size());

			// Step 2B. Loop through the batches.
			for (int b = 0; b < batches.size() - 1; b++) {
				// Step 2B.1. Reset the deltas
				clear_deltas();
				outputs.clear();
				expected.clear();

				// Step 2B.2. For each position in the batch, do backpropagation
				for (int p = batches[b]; p < batches[b + 1]; p++) {
					// Load position and forward propagate
					load_position((*training_data)[p].network_input);
					int forwardprop_score = evaluate(false); // We're not updating incrementally, so we should do a full forward propagation

					// Step 2B.2A. Since we're training, we'll apply the sigmoid activation function to the output
					OUTPUT_LAYER.neurons[0] = sigmoid(OUTPUT_LAYER.neurons[0]);

					outputs.push_back(static_cast<double>(forwardprop_score));

					// Back-propagate
					do_backprop(sigmoid((*training_data)[p].score));
					expected.push_back(static_cast<double>((*training_data)[p].score));
				}

				// Step 2B.3. Take the average of all the deltas and update the weights
				take_avg_deltas(batches[b + 1] - batches[b]);
				update_weights();

				// Step 2B.4. Output this batch's error.
				int width = 30;
				int right_padding = (double(b) / double(batches.size())) * width;
				int left_padding = width - right_padding;

				double loss_MSE = compute_loss<LOSS_F::MSE>(outputs, expected);
				double loss_AAE = compute_loss<LOSS_F::AAE>(outputs, expected);

				std::string progress(right_padding, '=');
				std::cout << "[" << b + 1 << "/" << batches.size() << "][" << progress << ">" << std::string(left_padding, ' ') << "] "
					<< "MSE:	" << loss_MSE << "	AAE:	" << loss_AAE << std::endl;

			}

			// Step 2C. When validation split is implemented, output the score of that here.
		}
	}



	/*
	
	When we compute the deltas, we calculate the sum of each individual delta from each data point. Therefore this method is used to take the average of all deltas.
	
	*/
	void Trainer::take_avg_deltas(size_t current_batch_size) {

		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			FIRST_HIDDEN_DELTAS[i] /= static_cast<double>(current_batch_size);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			SECOND_HIDDEN_DELTAS[i] /= static_cast<double>(current_batch_size);
			THIRD_HIDDEN_DELTAS[i] /= static_cast<double>(current_batch_size);
		}

		OUTPUT_DELTA /= static_cast<double>(current_batch_size);
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

	void Trainer::clear_delta_changes() {
		FIRST_HIDDEN_DELTAS_CHANGES.fill(0.0);
		SECOND_HIDDEN_DELTAS_CHANGES.fill(0.0);
		THIRD_HIDDEN_DELTAS_CHANGES.fill(0.0);
		OUTPUT_DELTA_CHANGE = 0.0;
	}
}