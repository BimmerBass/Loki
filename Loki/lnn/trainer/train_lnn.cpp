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

	Trainer::Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss, size_t _threads, double lRate, double lRate_decay)
		: epochs(_epochs), batch_size(_batch_size), loss_function(loss), threads(_threads), initial_learning_rate(lRate), learning_rate_decay(lRate_decay) {

		// Step 1. Make sure all hyperparameters are in their proper ranges
		try {
			if (dataset == "") { throw("Dataset should contain the path to a CSV file."); }
			if (_epochs <= 0) { throw("Epochs should be a positive number."); }
			if (_batch_size <= 0) { throw("Batch size should be a positive number."); }
			if (_threads <= 0) { throw("Threads should be a positive number."); }
			if (lRate <= 0) { throw("Learning rate should be a positive number."); }
			if (lRate_decay <= 0) { throw("Learning rate decay should be a positive number."); }
		}
		catch (const char* msg) { // If the hyperparameters aren't configured properly, abort
			std::cout << "[!] Exception thrown by Trainer::Trainer(): " << msg << std::endl;
			abort();
		}
		// Step 2. Allocate a vector for the dataset and a vector of deltas with size _threads.
		training_data = new std::vector<TrainingPosition>;
		
		for (int i = 0; i < threads; i++) {
			deltas.push_back(new Deltas);
		}

		// Step 3. Load the dataset
		load_dataset(dataset);
	}

	Trainer::~Trainer() {
		if (training_data != nullptr) {
			delete training_data;
		}
		// Delete all Deltas objects
		for (int i = 0; i < deltas.size(); i++) {
			if (deltas[i] != nullptr) { delete deltas[i]; }
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


	/*
	
	Multithreading and forward propagation presents a problem with saving the neurons's activations. Therefore the Deltas struct has its own containers
		for the neurons, so we'll need a forward propagation function that writes to these instead of to the real network neurons
	
	*/
	int Trainer::thread_evaluator(int thread_id) {

		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			dot_product<neuron_t, INPUT_SIZE>(deltas[thread_id]->INPUT_NEURONS, INPUT_LAYER.weights[i], deltas[thread_id]->FIRST_HIDDEN_NEURONS[i]);

			deltas[thread_id]->FIRST_HIDDEN_NEURONS[i] += FIRST_HIDDEN.biases[i];
			deltas[thread_id]->FIRST_HIDDEN_NEURONS[i] = apply_ReLU<neuron_t>(deltas[thread_id]->FIRST_HIDDEN_NEURONS[i]);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, FIRST_HIDDEN_SIZE>(deltas[thread_id]->FIRST_HIDDEN_NEURONS, FIRST_HIDDEN.weights[i], deltas[thread_id]->SECOND_HIDDEN_NEURONS[i]);

			deltas[thread_id]->SECOND_HIDDEN_NEURONS[i] += SECOND_HIDDEN.biases[i];
			deltas[thread_id]->SECOND_HIDDEN_NEURONS[i] = apply_ReLU<neuron_t>(deltas[thread_id]->SECOND_HIDDEN_NEURONS[i]);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, HIDDEN_STD_SIZE>(deltas[thread_id]->SECOND_HIDDEN_NEURONS, SECOND_HIDDEN.weights[i], deltas[thread_id]->THIRD_HIDDEN_NEURONS[i]);

			deltas[thread_id]->THIRD_HIDDEN_NEURONS[i] += THIRD_HIDDEN.biases[i];
			deltas[thread_id]->THIRD_HIDDEN_NEURONS[i] = apply_ReLU<neuron_t>(deltas[thread_id]->THIRD_HIDDEN_NEURONS[i]);
		}

		dot_product<neuron_t, HIDDEN_STD_SIZE>(deltas[thread_id]->THIRD_HIDDEN_NEURONS, THIRD_HIDDEN.weights[0], deltas[thread_id]->OUTPUT_NEURON);

		return std::max(-LNN::OUTPUT_BOUND, std::min(LNN::OUTPUT_BOUND, static_cast<int>(deltas[thread_id]->OUTPUT_NEURON)));
	}

	/*
	
	Backpropagation. When doing forward propagation of a single dataset, we need to propagate the error backwards in the network in order to appropriately update
		each weight and bias according to it's specific contribution to the error.
	
	*/

	void Trainer::do_backprop(volatile double expected_value, int thread_id) {

		// Step 1. Reset the changes in the deltas
		deltas[thread_id]->clear_delta_changes();

		// Step 2. Compute the error for the output value.
		// Note: Since the output doesn't use an activation function in LNN, we don't need to multiply any derivative of such function. This would normally be needed.
		double diff = double(OUTPUT_LAYER.neurons[0]) - expected_value;
		deltas[thread_id]->OUTPUT_DELTA_CHANGE = sigmoid_derivative(OUTPUT_LAYER.neurons[0]);

		if (loss_function == LOSS_F::AAE) {
			deltas[thread_id]->OUTPUT_DELTA_CHANGE *= (diff > 0) ? 1.0 : -1.0;
		}
		else {
			deltas[thread_id]->OUTPUT_DELTA_CHANGE *= 2 * (diff);
		}

		deltas[thread_id]->OUTPUT_DELTA += deltas[thread_id]->OUTPUT_DELTA_CHANGE;

		// Step 3. Now we can calculate the deltas for the third hidden layer
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			deltas[thread_id]->THIRD_HIDDEN_DELTAS_CHANGES[n] 
				= static_cast<double>(THIRD_HIDDEN.weights[0][n]) * deltas[thread_id]->OUTPUT_DELTA_CHANGE * ReLU_derivate(THIRD_HIDDEN.neurons[n]);
			deltas[thread_id]->THIRD_HIDDEN_DELTAS[n] += deltas[thread_id]->THIRD_HIDDEN_DELTAS_CHANGES[n];
		}

		// Step 4. Calculate the second hidden layer's deltas.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) { // Each neuron in third hidden layer
			for (int n = 0; n < HIDDEN_STD_SIZE; n++) { // Each neuron in second hidden layer
				deltas[thread_id]->SECOND_HIDDEN_DELTAS_CHANGES[n] 
					+= static_cast<double>(SECOND_HIDDEN.weights[m][n]) * deltas[thread_id]->THIRD_HIDDEN_DELTAS_CHANGES[m];
			}
		}

		// Step 4A. Multiply all the deltas with the activation function derivative
		for (int n = 0; n < HIDDEN_STD_SIZE; n++) {
			deltas[thread_id]->SECOND_HIDDEN_DELTAS_CHANGES[n] *= ReLU_derivate(SECOND_HIDDEN.neurons[n]);
			deltas[thread_id]->SECOND_HIDDEN_DELTAS[n] += deltas[thread_id]->SECOND_HIDDEN_DELTAS_CHANGES[n];
		}

		// Step 5. Lastly, calculate the first hidden layer's deltas.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
			for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
				deltas[thread_id]->FIRST_HIDDEN_DELTAS_CHANGES[n] 
					+= static_cast<double>(FIRST_HIDDEN.weights[m][n]) * deltas[thread_id]->SECOND_HIDDEN_DELTAS_CHANGES[m];
			}
		}

		// Step 5A. Multiply these deltas by the activation function derivatives and we're done :))
		for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
			deltas[thread_id]->FIRST_HIDDEN_DELTAS_CHANGES[n] *= ReLU_derivate(FIRST_HIDDEN.neurons[n]);
			deltas[thread_id]->FIRST_HIDDEN_DELTAS[n] += deltas[thread_id]->FIRST_HIDDEN_DELTAS_CHANGES[n];
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
			weight_gradient = THIRD_HIDDEN.neurons[n] * main_deltas.OUTPUT_DELTA;
			bias_gradient = main_deltas.OUTPUT_DELTA;

			// Update the weight and bias.
			THIRD_HIDDEN.weights[0][n] -= learning_rate * weight_gradient;
			THIRD_HIDDEN.biases[n] -= learning_rate * bias_gradient;
		}

		// Step 2. Update the weights and biases of the second hidden layer.
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) { // For each neuron in the third hidden layer
			for (int n = 0; n < HIDDEN_STD_SIZE; n++) { // For each neuron in the second hidden layer
				weight_gradient = SECOND_HIDDEN.neurons[n] * main_deltas.THIRD_HIDDEN_DELTAS[m];
				bias_gradient = main_deltas.THIRD_HIDDEN_DELTAS[m];

				// Update the weight and bias
				SECOND_HIDDEN.weights[m][n] -= learning_rate * weight_gradient;
				SECOND_HIDDEN.biases[n] -= learning_rate * bias_gradient;
			}
		}

		// Step 3. Update weights and biases for the first hidden layer
		for (int m = 0; m < HIDDEN_STD_SIZE; m++) {
			for (int n = 0; n < FIRST_HIDDEN_SIZE; n++) {
				weight_gradient = FIRST_HIDDEN.neurons[n] * main_deltas.SECOND_HIDDEN_DELTAS[m];
				bias_gradient = main_deltas.SECOND_HIDDEN_DELTAS[m];

				// Update the weight and the bias
				FIRST_HIDDEN.weights[m][n] -= learning_rate * weight_gradient;
				FIRST_HIDDEN.biases[n] -= learning_rate * bias_gradient;
			}
		}

		// Step 4. Lastly, update the weights from the input to the first hidden layer.
		// Note: The input shouldn't have any bias, so this won't be updated.
		for (int m = 0; m < FIRST_HIDDEN_SIZE; m++) {
			for (int n = 0; n < INPUT_SIZE; n++) {
				weight_gradient = INPUT_LAYER.neurons[n] * main_deltas.FIRST_HIDDEN_DELTAS[m];
				
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
	
	Helper function to combine two vectors
	
	*/
	template<typename T>
	std::vector<T> combine_vectors(const std::vector<std::vector<T>>& v) {
		std::vector<T> output;

		for (int i = 0; i < v.size(); i++) {
			for (int j = 0; j < v[i].size(); j++) {
				output.push_back(v[i][j]);
			}
		}
		
		return output;
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

		// The threads will write to these and they will then be combined in the outputs and expected vectors
		std::vector<std::vector<double>> threads_outputs;
		std::vector<std::vector<double>> threads_expected;

		for (int i = 0; i < threads; i++) {
			std::vector<double> tmp1, tmp2;
			threads_outputs.push_back(tmp1); threads_expected.push_back(tmp2);
		}

		// Vector for holding all threads while they're working
		std::vector<std::thread> workers;

		// Vector to hold all the sub-batches used when optimizing with multiple threads
		std::vector<std::vector<TrainingPosition>> sub_batches;

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
				clear_all_deltas();
				outputs.clear();
				expected.clear();

				// Step 2B.2. Now divide the batch into sub-batches that the different threads will run.
				subdivide_batch(sub_batches, batches, b);
				assert(sub_batches.size() == threads);

				// Step 2B.3. Start up the threads
				workers.clear();
				for (int i = 0; i < threads; i++) {
					threads_outputs[i].clear();
					threads_expected[i].clear();

					workers.push_back(std::thread(&Trainer::thread_optimizer, this, 
													std::ref(sub_batches[i]), 
													std::ref(threads_outputs[i]),
													std::ref(threads_expected[i]), 
													i));
				}

				// Step 2B.4. Wait for the threads to join, and when they've all done that, combine their calculate the average deltas
				for (int i = 0; i < workers.size(); i++) {
					workers[i].join();
				}
				compute_main_deltas();


				// Step 2B.5. Take the average of all the deltas and update the weights
				//take_avg_deltas(batches[b + 1] - batches[b]);
				update_weights();

				// Step 2B.4. Combine the outputs/expected from the threads and output this batch's error.
				outputs = combine_vectors<double>(threads_outputs);
				expected = combine_vectors<double>(threads_expected);

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
	
	The following method specifies what each thread should do.
	
	*/
	void Trainer::thread_optimizer(const std::vector<TrainingPosition>& positions, std::vector<double>& outputs, std::vector<double>& expected, volatile int thread_id) {

		// Loop through all the training points
		for (int p = 0; p < positions.size(); p++) {
			// Step 1. Load the position and forward-propagate
			deltas[thread_id]->load_input(positions[p].network_input);
			int output = thread_evaluator(thread_id);

			// Take the sigmoid of the output neuron
			deltas[thread_id]->OUTPUT_NEURON = sigmoid(deltas[thread_id]->OUTPUT_NEURON);
			outputs.push_back(static_cast<double>(deltas[thread_id]->OUTPUT_NEURON));

			// Back propagate
			do_backprop(positions[p].score, thread_id);
			expected.push_back(static_cast<double>(positions[p].score));
		}

		// Step 2. Average out all deltas
		deltas[thread_id]->take_avg_deltas(positions.size());
	}



	/*
	
	Helper method for compute_main_deltas. Adds one array to another.
	
	*/
	template<typename T, size_t SIZE>
	void array_add(std::array<T, SIZE>& _Dst, const std::array<T, SIZE>& _Src) {

		for (size_t i = 0; i < SIZE; i++) {
			_Dst[i] += _Src[i];
		}

	}

	/*
	
	Another helper method. Divides all elements in an array by a number
	
	*/
	template<typename T, size_t SIZE>
	void divide_array(std::array<T, SIZE>& _Dst, const T x) {
		for (size_t i = 0; i < SIZE; i++) {
			_Dst[i] /= x;
		}
	}

	/*
	
	When we're done computing the deltas for each thread, we will take the average of these and insert the values into the main_deltas object.
		NOTE: main_deltas should be cleared before doing this operation.
	*/
	void Trainer::compute_main_deltas() {
		assert(deltas.size() == threads);

		// Step 1. Add all the deltas from the vector to the main_deltas object.
		for (int d = 0; d < deltas.size(); d++) {

			// Add all arrays from deltas[i] to main_deltas
			array_add<double, FIRST_HIDDEN_SIZE>(main_deltas.FIRST_HIDDEN_DELTAS, deltas[d]->FIRST_HIDDEN_DELTAS);
			array_add<double, HIDDEN_STD_SIZE>(main_deltas.SECOND_HIDDEN_DELTAS, deltas[d]->SECOND_HIDDEN_DELTAS);
			array_add<double, HIDDEN_STD_SIZE>(main_deltas.THIRD_HIDDEN_DELTAS, deltas[d]->THIRD_HIDDEN_DELTAS);
			main_deltas.OUTPUT_DELTA += deltas[d]->OUTPUT_DELTA;
		}

		// Step 2. Now divide this by the amount of threads we're using
		divide_array<double, FIRST_HIDDEN_SIZE>(main_deltas.FIRST_HIDDEN_DELTAS, static_cast<double>(threads));
		divide_array<double, HIDDEN_STD_SIZE>(main_deltas.SECOND_HIDDEN_DELTAS, static_cast<double>(threads));
		divide_array<double, HIDDEN_STD_SIZE>(main_deltas.THIRD_HIDDEN_DELTAS, static_cast<double>(threads));
		main_deltas.OUTPUT_DELTA /= static_cast<double>(threads);
	}


	/*
	
	For more organized code, the following method is used to clear all deltas in use.
	
	*/
	void Trainer::clear_all_deltas() {

		for (int i = 0; i < deltas.size(); i++) {
			deltas[i]->clear_deltas();
			deltas[i]->clear_delta_changes();
		}

		main_deltas.clear_deltas();
		main_deltas.clear_delta_changes();
	}

	/*
	
	When we compute the deltas, we calculate the sum of each individual delta from each data point. Therefore this method is used to take the average of all deltas.
	
	*/
	void Deltas::take_avg_deltas(size_t current_batch_size) {

		divide_array<double, FIRST_HIDDEN_SIZE>(FIRST_HIDDEN_DELTAS, static_cast<double>(current_batch_size));
		divide_array<double, HIDDEN_STD_SIZE>(SECOND_HIDDEN_DELTAS, static_cast<double>(current_batch_size));
		divide_array<double, HIDDEN_STD_SIZE>(THIRD_HIDDEN_DELTAS, static_cast<double>(current_batch_size));

		OUTPUT_DELTA /= static_cast<double>(current_batch_size);
	}

	/*
	
	Set all deltas to zero. This is done before each batch step.
	
	*/
	void Deltas::clear_deltas() {
		FIRST_HIDDEN_DELTAS.fill(0.0);
		SECOND_HIDDEN_DELTAS.fill(0.0);
		THIRD_HIDDEN_DELTAS.fill(0.0);
		OUTPUT_DELTA = 0.0;
	}

	void Deltas::clear_delta_changes() {
		FIRST_HIDDEN_DELTAS_CHANGES.fill(0.0);
		SECOND_HIDDEN_DELTAS_CHANGES.fill(0.0);
		THIRD_HIDDEN_DELTAS_CHANGES.fill(0.0);
		OUTPUT_DELTA_CHANGE = 0.0;
	}


	/*
	
	Divide a batch into work for different threads
	
	*/
	// Divides a batch into sub-batches for the threads
	void Trainer::subdivide_batch(std::vector<std::vector<TrainingPosition>>& sub_batches, const std::vector<size_t>& batch, int current) {
		// Step 1. Clear the sub batch vector and find the start/end index in the training_data vector of our current batch.
		assert(current < batch.size() - 1);
		sub_batches.clear();

		size_t start = batch[current];
		size_t end = batch[current + 1];


		int remainder = (end - start) % threads;
		int sub_batch_size = (end - start) / threads;

		// Step 2. Now subdivide all training positions into the different sub_batches
		for (int i = 0; i < threads; i++) {

			std::vector<TrainingPosition> sub_batch;

			// Step 2A. Add all positions to the sub_batch
			for (int n = i * sub_batch_size; n < (i + 1) * sub_batch_size; n++) {
				assert(start + n < training_data->size());

				sub_batch.push_back((*training_data)[start + n]);
			}
			sub_batches.push_back(sub_batch);
		}

		// Step 3. If there is a remainder, add this to the first thread
		if (remainder > 0) {
			for (int n = end - 1; n > end - 1 - remainder; n--) {
				sub_batches[0].push_back((*training_data)[n]);
			}
		}
		
	}
}