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


	/*
	
	Backpropagation: This is the core of neural network learning. When we forward propagate, the output will probably not be equal to our
		target. Therefore we need a way to find out how each weight influence the error we're measuring. This is what backprop is used for.

		We start by finding the cost of this single training example, C = (a - y)^2 (MSE) or C = |a - y| (AAE).
		Note: The output values of the neural network in this case use a linear activation σ(x) = x, but for training purposes
			the sigmoid function is used, which means that a = σ(O) and y = σ(Eval) (where O is net output and Eval is Loki's evaluation).
		For the output delta, we compute the derivatives:
			MSE: ∂C/∂z = 2 * (a - y) * σ'(z)
			AAE: ∂C/∂z = (|a - y|/(a - y))* σ'(z) (Note (|a - y|/(a - y)) = -1 if (a - y) < 0 and +1 if (a - y) > 0, approximated as >= here)
		Now we can iteratively (not in this instance, but in principle) go back in the network and calculate all deltas. If we are at layer l - 1,
		we then already know δ[l], and we want to know δ[l - 1] (assuming there is only one weight/connection).
		From the chain rule we know that ∂C/∂z[l-1] = ∂C/∂z[l] * ∂z[l]/∂z[l-1]. Additionally we know that z[l] = sum(w[l][j] * z[l-1][j]), which
		means that for the j'th weight the delta becomes:
		δ[l - 1] = ∂C/∂z[l] * ∂z[l]/∂z[l-1] = δ[l] * w[l][j]. Where j is the j'th neuron in layer l - 1.
		Note: We also have to multiply by ReLU'(z[l]) since if z[l] < 0, it shouldn't be counted as contributing to the error.
	*/
	void Trainer::back_propagation(int thread_id, int target_output) {
		// Step 1. Clear all deltas.
		thread_data[thread_id]->clear_deltas();

		// Step 2. Calculate the delta for the output.
		double diff = (sigmoid(thread_data[thread_id]->OUTPUT_NEURON) - sigmoid(target_output));
		thread_data[thread_id]->OUTPUT_DELTA = sigmoid_derivative(thread_data[thread_id]->OUTPUT_NEURON);

		if (loss_function == LOSS_F::AAE) {
			thread_data[thread_id]->OUTPUT_DELTA *= (diff > 0 ? 1.0 : ((diff < 0) ? -1.0 : 0.0));
		}
		else {
			thread_data[thread_id]->OUTPUT_DELTA *= 2 * diff;
		}

		// Step 3. Now calculate the deltas in the third hidden layer.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			thread_data[thread_id]->THIRD_HIDDEN_DELTAS[i] = thread_data[thread_id]->OUTPUT_DELTA * THIRD_HIDDEN.weights[0][i];
			thread_data[thread_id]->THIRD_HIDDEN_DELTAS[i] *= ReLU_derivate(thread_data[thread_id]->THIRD_HIDDEN_NEURONS[i]);
		}

		// Step 4. Calculate the deltas in the second hidden layer.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) { // For each neuron in the next layer
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				thread_data[thread_id]->SECOND_HIDDEN_DELTAS[j] += thread_data[thread_id]->THIRD_HIDDEN_DELTAS[i] * SECOND_HIDDEN.weights[i][j];
			}
		}
		// Step 4A. Apply the relu derivatives
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			thread_data[thread_id]->SECOND_HIDDEN_DELTAS[i] *= ReLU_derivate(thread_data[thread_id]->SECOND_HIDDEN_NEURONS[i]);
		}

		// Step 5. Do the same for the second hidden layer back to the first
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
				thread_data[thread_id]->FIRST_HIDDEN_DELTAS[j] += thread_data[thread_id]->SECOND_HIDDEN_DELTAS[i] * FIRST_HIDDEN.weights[i][j];
			}
		}
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			thread_data[thread_id]->FIRST_HIDDEN_DELTAS[i] *= ReLU_derivate(thread_data[thread_id]->FIRST_HIDDEN_NEURONS[i]);
		}

		// Step 6. Update the gradients, and we're done
		thread_data[thread_id]->update_gradients();
	}


	/*
	
	Gradient increment: When we have calculated all deltas, we need to update our gradients.
		This is quite easy. If we are at layer l and we want to update the weight connecting the i'th neuron in l to the j'th neuron in l - 1,
		we need to find the expression for ∂C/∂w[l -1]i,j, which, from the chain rule, can be given as:
			∂C/∂w[l-1],i,j = ∂C/∂z[l]i * ∂z[l]i/∂w[l-1]i,j = δ[l]i * a[l-1]j
		Additionally we want to find ∂C/∂b[l]i, which is:
			∂C/∂b[l]i = δ[l]i
	*/
	void ThreadData::update_gradients() {

		// Step 1. Compute the gradients of the weight from the input to the first hidden layer.
		// Note: The inputs shouldn't have any biases, so these are left out. Additionally, the input has no activation function, so we don't need to apply that here.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < INPUT_SIZE; j++) {
				// Weight gradient
				INPUT_WEIGHT_GRADIENTS[i][j] += FIRST_HIDDEN_DELTAS[i] * INPUT_NEURONS[j];
			}
		}

		// Step 2. Now compute the gradients of the weights and biases from the first hidden layer to the second hidden layer.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				// Weight gradient. 
				// Note: We need to apply the ReLU activation to to neuron.
				FIRST_HIDDEN_WEIGHT_GRADIENTS[j][i] += SECOND_HIDDEN_DELTAS[j] * ReLU(FIRST_HIDDEN_NEURONS[i]);
			}

			// Bias gradient
			FIRST_HIDDEN_BIAS_GRADIENTS[i] += FIRST_HIDDEN_DELTAS[i];
		}

		// Step 3. Do the same for the gradients from the second layer to the third layer.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				SECOND_HIDDEN_WEIGHT_GRADIENTS[j][i] += THIRD_HIDDEN_DELTAS[j] * ReLU(SECOND_HIDDEN_NEURONS[i]);
			}

			// Bias gradient
			SECOND_HIDDEN_BIAS_GRADIENTS[i] += SECOND_HIDDEN_DELTAS[i];
		}

		// Step 4. Compute the gradients for the third layer
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			// Weight gradient
			THIRD_HIDDEN_WEIGHT_GRADIENTS[i] += OUTPUT_DELTA * ReLU(THIRD_HIDDEN_NEURONS[i]);

			// Bias gradient
			THIRD_HIDDEN_BIAS_GRADIENTS[i] += OUTPUT_DELTA;
		}
	}


	/*
	
	Calculate the average of all the threads's gradients and insert them into main_thread_data.
	
	*/
	void Trainer::compute_average_gradients() {
		// Step 1. Clear the main thread
		main_thread_data->clear_gradients();
		assert(thread_count == thread_data.size());

		// Step 2. Add all the gradients
		for (int t = 0; t < thread_count; t++) {

			for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
				add_array<double, INPUT_SIZE>(main_thread_data->INPUT_WEIGHT_GRADIENTS[i], thread_data[t]->INPUT_WEIGHT_GRADIENTS[i]);
			}

			for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
				add_array<double, FIRST_HIDDEN_SIZE>(main_thread_data->FIRST_HIDDEN_WEIGHT_GRADIENTS[i], thread_data[t]->FIRST_HIDDEN_WEIGHT_GRADIENTS[i]);
				add_array<double, HIDDEN_STD_SIZE>(main_thread_data->SECOND_HIDDEN_WEIGHT_GRADIENTS[i], thread_data[t]->SECOND_HIDDEN_WEIGHT_GRADIENTS[i]);
			}

			add_array<double, HIDDEN_STD_SIZE>(main_thread_data->THIRD_HIDDEN_WEIGHT_GRADIENTS, thread_data[t]->THIRD_HIDDEN_WEIGHT_GRADIENTS);

			add_array<double, FIRST_HIDDEN_SIZE>(main_thread_data->FIRST_HIDDEN_BIAS_GRADIENTS, thread_data[t]->FIRST_HIDDEN_BIAS_GRADIENTS);
			add_array<double, HIDDEN_STD_SIZE>(main_thread_data->SECOND_HIDDEN_BIAS_GRADIENTS, thread_data[t]->SECOND_HIDDEN_BIAS_GRADIENTS);
			add_array<double, HIDDEN_STD_SIZE>(main_thread_data->THIRD_HIDDEN_BIAS_GRADIENTS, thread_data[t]->THIRD_HIDDEN_BIAS_GRADIENTS);
		}

		// Step 3. Now take the average of all gradients by dividing the elements by the number of threads
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			divide_array<double, INPUT_SIZE>(main_thread_data->INPUT_WEIGHT_GRADIENTS[i], static_cast<double>(thread_count));
		}
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			divide_array<double, FIRST_HIDDEN_SIZE>(main_thread_data->FIRST_HIDDEN_WEIGHT_GRADIENTS[i], static_cast<double>(thread_count));
			divide_array<double, HIDDEN_STD_SIZE>(main_thread_data->SECOND_HIDDEN_WEIGHT_GRADIENTS[i], static_cast<double>(thread_count));
		}
		divide_array<double, HIDDEN_STD_SIZE>(main_thread_data->THIRD_HIDDEN_WEIGHT_GRADIENTS, static_cast<double>(thread_count));

		divide_array<double, FIRST_HIDDEN_SIZE>(main_thread_data->FIRST_HIDDEN_BIAS_GRADIENTS, static_cast<double>(thread_count));
		divide_array<double, HIDDEN_STD_SIZE>(main_thread_data->SECOND_HIDDEN_BIAS_GRADIENTS, static_cast<double>(thread_count));
		divide_array<double, HIDDEN_STD_SIZE>(main_thread_data->THIRD_HIDDEN_BIAS_GRADIENTS, static_cast<double>(thread_count));
	}


	/*
	
	Parameter updates: When we have computed the gradient of the loss function w.r.t. the parameters, we have an expression for the 
		"direction" of highest increase. This means that we should subtract part of the gradients, which for parameter p, at
		iteration i will be:
			p[i + 1] = p[i] - η * g[p] (gradient w.r.t. p) where η is the learning rate
		This is what the following method is used for.
	*/
	void Trainer::update_parameters() {

		// Step 1. Update all weights from input to first hidden layer
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < INPUT_SIZE; j++) {
				INPUT_LAYER.weights[i][j] -= learning_rate * main_thread_data->INPUT_WEIGHT_GRADIENTS[i][j];
			}
		}

		// Step 2. Update weights and biases of first hidden layer to second hidden layer.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				FIRST_HIDDEN.weights[j][i] -= learning_rate * main_thread_data->FIRST_HIDDEN_WEIGHT_GRADIENTS[j][i];
			}

			// Update bias
			FIRST_HIDDEN.biases[i] -= learning_rate * main_thread_data->FIRST_HIDDEN_BIAS_GRADIENTS[i];
		}

		// Step 3. Update weights an biases of second hidden layer to third hidden layer
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++){
				SECOND_HIDDEN.weights[j][i] -= learning_rate * main_thread_data->SECOND_HIDDEN_WEIGHT_GRADIENTS[j][i];
			}

			// Update bias
			SECOND_HIDDEN.biases[i] -= learning_rate * main_thread_data->SECOND_HIDDEN_BIAS_GRADIENTS[i];
		}

		// Step 4. Update weights and biases from third hidden layer to the output, and we're done.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++){
			// Weight update
			THIRD_HIDDEN.weights[0][i] -= learning_rate * main_thread_data->THIRD_HIDDEN_WEIGHT_GRADIENTS[i];

			// Bias update
			THIRD_HIDDEN.biases[i] -= learning_rate * main_thread_data->THIRD_HIDDEN_BIAS_GRADIENTS[i];
		}
	}





	/*
	Below are non functional helper functions.
	*/

	// Set the input of the network
	void ThreadData::set_input(const std::array<int8_t, INPUT_SIZE>& input) {
		INPUT_NEURONS.fill(0);

		for (int i = 0; i < INPUT_SIZE; i++) {
			INPUT_NEURONS[i] = static_cast<neuron_t>(input[i]);
		}
	}

	// When we calculate the gradients, we sum them for each data point in the batch. These need to be averaged.
	void ThreadData::average_gradients(size_t batch_size) {

		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			divide_array<double, INPUT_SIZE>(INPUT_WEIGHT_GRADIENTS[i], static_cast<double>(batch_size));
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			divide_array<double, FIRST_HIDDEN_SIZE>(FIRST_HIDDEN_WEIGHT_GRADIENTS[i], static_cast<double>(batch_size));
			divide_array<double, HIDDEN_STD_SIZE>(SECOND_HIDDEN_WEIGHT_GRADIENTS[i], static_cast<double>(batch_size));
		}

		divide_array<double, HIDDEN_STD_SIZE>(THIRD_HIDDEN_WEIGHT_GRADIENTS, static_cast<double>(batch_size));

		divide_array<double, FIRST_HIDDEN_SIZE>(FIRST_HIDDEN_BIAS_GRADIENTS, static_cast<double>(batch_size));
		divide_array<double, HIDDEN_STD_SIZE>(SECOND_HIDDEN_BIAS_GRADIENTS, static_cast<double>(batch_size));
		divide_array<double, HIDDEN_STD_SIZE>(THIRD_HIDDEN_BIAS_GRADIENTS, static_cast<double>(batch_size));
	}

	// Set all gradients to 0.
	void ThreadData::clear_gradients() {
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			INPUT_WEIGHT_GRADIENTS[i].fill(0);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			FIRST_HIDDEN_WEIGHT_GRADIENTS[i].fill(0);
			SECOND_HIDDEN_WEIGHT_GRADIENTS[i].fill(0);
		}

		THIRD_HIDDEN_WEIGHT_GRADIENTS.fill(0);

		FIRST_HIDDEN_BIAS_GRADIENTS.fill(0);
		SECOND_HIDDEN_BIAS_GRADIENTS.fill(0);
		THIRD_HIDDEN_BIAS_GRADIENTS.fill(0);
	}


	// Set all deltas to zero
	void ThreadData::clear_deltas() {
		FIRST_HIDDEN_DELTAS.fill(0);
		SECOND_HIDDEN_DELTAS.fill(0);
		THIRD_HIDDEN_DELTAS.fill(0);
		OUTPUT_DELTA = 0;
	}
}