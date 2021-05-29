#include "train_lnn.h"



namespace Training {

	/*
	Constructor. Load the dataset, allocate all neccesarry objects on heap and set hyperparameters.
	*/
	Trainer::Trainer(std::string datafile, size_t _epochs, size_t _batch_size, LOSS_F _loss, size_t _threads, 
		double eta_start, double eta_decay, double _min, double _max, LNN::LNN_FileType _sf, std::string _out)
		: epochs(_epochs), batch_size(_batch_size), loss_function(_loss), thread_count(_threads), 
		initial_learning_rate(eta_start), learning_rate_decay(eta_decay), parameter_min_val(_min), parameter_max_val(_max), save_format(_sf), output_filename(_out) {

		// Step 1. Make sure all hyperparameters are in their proper ranges
		try {
			if (datafile == "") { throw("Dataset must contain the path to a CSV file."); }
			if (_epochs <= 0) { throw("Epochs must be a positive number."); }
			if (_batch_size <= 0) { throw("Batch size must be a positive number."); }
			if (_threads <= 0) { throw("Threads must be a positive number."); }
			if (eta_start <= 0) { throw("Learning rate must be a positive number."); }
			if (eta_decay <= 0) { throw("Learning rate decay must be a positive number."); }
			if (_min >= _max) { throw("Minimum parameter initialization value must be smaller than maximum parameter initialization value."); }
			if ((_sf == LNN::BIN && output_filename.find(".csv") != std::string::npos) ||
				(_sf == LNN::CSV && output_filename.find(".lnn") != std::string::npos)) {
				throw("Output file format must match the filename specified.");
			}
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
		adam_momentum = new Adam::AdamParameters;

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
		if (adam_momentum != nullptr) { delete adam_momentum; }
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

	Saving the model. This can be done in two ways: CSV or BIN.
		CSV: Will write all weights and biases to a csv file in text format.
		BIN: Will write all weights and biases to a ".lnn" file in binary format.
	
	*/
	template<>
	void Trainer::save_model<LNN::CSV>() {
		
		// Step 1. If no output name has been given, create one based on the date and time
		std::string output_file = output_filename;

		if (output_filename == "") {
			output_file = "LokiNet-" + getDateTime() + ".csv";
		}

		// Step 2. Open the file.
		std::ofstream csv_file(output_file);

		// Step 3. Write all biases from the first hidden layer to the third in the first three rows.
		write_array<neuron_t, FIRST_HIDDEN_SIZE>(csv_file, FIRST_HIDDEN.biases);
		write_array<neuron_t, HIDDEN_STD_SIZE>(csv_file, SECOND_HIDDEN.biases);
		write_array<neuron_t, HIDDEN_STD_SIZE>(csv_file, THIRD_HIDDEN.biases);

		// Step 4. Write all weights to the file.
		write_multiple_arrays<neuron_t, INPUT_SIZE, FIRST_HIDDEN_SIZE>(csv_file, INPUT_LAYER.weights);
		write_multiple_arrays<neuron_t, FIRST_HIDDEN_SIZE, HIDDEN_STD_SIZE>(csv_file, FIRST_HIDDEN.weights);
		write_multiple_arrays<neuron_t, HIDDEN_STD_SIZE, HIDDEN_STD_SIZE>(csv_file, SECOND_HIDDEN.weights);
		write_multiple_arrays<neuron_t, HIDDEN_STD_SIZE, OUTPUT_SIZE>(csv_file, THIRD_HIDDEN.weights);

		// Step 5. Close the file.
		csv_file.close();

		std::cout << "Saved model at " << output_file << std::endl;
	}

	template<>
	void Trainer::save_model<LNN::BIN>() {

		// Step 1. If no output name has been given, create one based on the date and time
		std::string output_file = output_filename;

		if (output_filename == "") {
			output_file = "LokiNet-" + getDateTime() + ".lnn";
		}

		// Step 2. Open the file.
		FILE* bFile = nullptr;

#if (defined(_WIN32) || defined(_WIN64))
		fopen_s(&bFile, output_file.c_str(), "wb");
#else
		bFile = fopen(output_file.c_str(), "wb");
#endif

		// Step 3. Write the input weights.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			fwrite(INPUT_LAYER.weights[i].data(), sizeof(neuron_t), INPUT_SIZE, bFile);
		}

		// Step 4. Write the biases of the first hidden layer and then its weights
		fwrite(FIRST_HIDDEN.biases.data(), sizeof(neuron_t), FIRST_HIDDEN_SIZE, bFile);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			fwrite(FIRST_HIDDEN.weights[i].data(), sizeof(neuron_t), FIRST_HIDDEN_SIZE, bFile);
		}

		// Step 5. Write all biases of the second hidden layer and then its weights
		fwrite(SECOND_HIDDEN.biases.data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			fwrite(SECOND_HIDDEN.weights[i].data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);
		}

		// Step 6. Write all biases of the third hidden layer and then its weights
		fwrite(THIRD_HIDDEN.biases.data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);
		fwrite(THIRD_HIDDEN.weights[0].data(), sizeof(neuron_t), HIDDEN_STD_SIZE, bFile);

		// Step 7. Close the file
		fclose(bFile);

		std::cout << "Saved model at " << output_file << std::endl;
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


		return static_cast<double>(thread_data[thread_id]->OUTPUT_NEURON);
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
	void Trainer::update_parameters(int current_epoch) {
		volatile double gradient = 0.0;

		// Step 1. Update all weights from input to first hidden layer
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < INPUT_SIZE; j++) {
				// Update the adam parameters.
				adam_momentum->INPUT_WEIGHTS[i].update(j, main_thread_data->INPUT_WEIGHT_GRADIENTS[i][j]);

				gradient = adam_momentum->INPUT_WEIGHTS[i].m_hat(j, current_epoch) / (std::sqrt(adam_momentum->INPUT_WEIGHTS[i].v_hat(j, current_epoch)) + Adam::EPSILON);
				
				// Now update the parameter.
				INPUT_LAYER.weights[i][j] -= learning_rate * gradient;

				//INPUT_LAYER.weights[i][j] -= learning_rate * main_thread_data->INPUT_WEIGHT_GRADIENTS[i][j];
			}
		}

		// Step 2. Update weights and biases of first hidden layer to second hidden layer.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				adam_momentum->FIRST_HIDDEN_WEIGHTS[j].update(i, main_thread_data->FIRST_HIDDEN_WEIGHT_GRADIENTS[j][i]);
				gradient = adam_momentum->FIRST_HIDDEN_WEIGHTS[j].m_hat(i, current_epoch) /
					(std::sqrt(adam_momentum->FIRST_HIDDEN_WEIGHTS[j].v_hat(i, current_epoch)) + Adam::EPSILON);

				FIRST_HIDDEN.weights[j][i] -= learning_rate * gradient;
				//FIRST_HIDDEN.weights[j][i] -= learning_rate * main_thread_data->FIRST_HIDDEN_WEIGHT_GRADIENTS[j][i];
			}

			// Update bias
			adam_momentum->FIRST_HIDDEN_BIAS.update(i, main_thread_data->FIRST_HIDDEN_BIAS_GRADIENTS[i]);
			gradient = adam_momentum->FIRST_HIDDEN_BIAS.m_hat(i, current_epoch) / (std::sqrt(adam_momentum->FIRST_HIDDEN_BIAS.v_hat(i, current_epoch)) + Adam::EPSILON);
			
			FIRST_HIDDEN.biases[i] -= learning_rate * gradient;
			//FIRST_HIDDEN.biases[i] -= learning_rate * main_thread_data->FIRST_HIDDEN_BIAS_GRADIENTS[i];
		}

		// Step 3. Update weights an biases of second hidden layer to third hidden layer
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++){
				adam_momentum->SECOND_HIDDEN_WEIGHTS[j].update(i, main_thread_data->SECOND_HIDDEN_WEIGHT_GRADIENTS[j][i]);
				gradient = adam_momentum->SECOND_HIDDEN_WEIGHTS[j].m_hat(i, current_epoch) /
					(std::sqrt(adam_momentum->SECOND_HIDDEN_WEIGHTS[j].v_hat(i, current_epoch)) + Adam::EPSILON);

				SECOND_HIDDEN.weights[j][i] -= learning_rate * gradient;
				//SECOND_HIDDEN.weights[j][i] -= learning_rate * main_thread_data->SECOND_HIDDEN_WEIGHT_GRADIENTS[j][i];
			}

			// Update bias
			adam_momentum->SECOND_HIDDEN_BIAS.update(i, main_thread_data->SECOND_HIDDEN_BIAS_GRADIENTS[i]);
			gradient = adam_momentum->SECOND_HIDDEN_BIAS.m_hat(i, current_epoch) / (std::sqrt(adam_momentum->SECOND_HIDDEN_BIAS.v_hat(i, current_epoch)) + Adam::EPSILON);
			
			SECOND_HIDDEN.biases[i] -= learning_rate * gradient;
			//SECOND_HIDDEN.biases[i] -= learning_rate * main_thread_data->SECOND_HIDDEN_BIAS_GRADIENTS[i];
		}

		// Step 4. Update weights and biases from third hidden layer to the output, and we're done.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++){
			// Weight update
			adam_momentum->THIRD_HIDDEN_WEIGHTS.update(i, main_thread_data->THIRD_HIDDEN_WEIGHT_GRADIENTS[i]);
			gradient = adam_momentum->THIRD_HIDDEN_WEIGHTS.m_hat(i, current_epoch) / (std::sqrt(adam_momentum->THIRD_HIDDEN_WEIGHTS.v_hat(i, current_epoch)) + Adam::EPSILON);
			
			THIRD_HIDDEN.weights[0][i] -= learning_rate * gradient;
			//THIRD_HIDDEN.weights[0][i] -= learning_rate * main_thread_data->THIRD_HIDDEN_WEIGHT_GRADIENTS[i];

			// Bias update
			//THIRD_HIDDEN.biases[i] -= learning_rate * main_thread_data->THIRD_HIDDEN_BIAS_GRADIENTS[i];
			adam_momentum->THIRD_HIDDEN_BIAS.update(i, main_thread_data->THIRD_HIDDEN_BIAS_GRADIENTS[i]);
			gradient = adam_momentum->THIRD_HIDDEN_BIAS.m_hat(i, current_epoch) / (std::sqrt(adam_momentum->THIRD_HIDDEN_BIAS.v_hat(i, current_epoch)) + Adam::EPSILON);

			THIRD_HIDDEN.biases[i] -= learning_rate * gradient;
		}

	}

	
	/*
	
	Thread optimizer. This function is what each thread will run on its portion of the batch.
	
	*/
	void Trainer::run_thread(const std::vector<TrainingPosition>& positions, std::vector<double>& outputs, std::vector<double>& expected, int thread_id) {
		// Step 1. Clear the data vectors and the gradients.
		outputs.clear();
		expected.clear();
		thread_data[thread_id]->clear_gradients();

		// Step 2. Loop through all positions
		for (int i = 0; i < positions.size(); i++) {
			// Step 2A. Load the position and forward propagate. Save the network's output in the outputs vector
			thread_data[thread_id]->set_input(positions[i].network_inputs);
			forward_propagate(thread_id);
			outputs.push_back(static_cast<double>(thread_data[thread_id]->OUTPUT_NEURON));

			// Step 2B. Backpropagate and save the expected score in the "expected" vector.
			// Note: The gradients will be updated together with the deltas in the back_propagation function.
			back_propagation(thread_id, positions[i].score);
			expected.push_back(static_cast<double>(positions[i].score));

		}

		// Step 3. After having calculated all the accumulated gradients, take the average over the training positions
		thread_data[thread_id]->average_gradients(positions.size());
	}

	/*
	
	Since we need to use the weights's and biases's values to compute new deltas, the network can't be zero-initialized. Therefore, this method will initialize all values 
		(except input/output biases which will be zero) to random values within some interval
	
	*/
	void Trainer::initialize_random() {
		// Step 1. Seed the PRNG.
		std::srand(std::time(0));

		// Step 2. Initialize the input weights
		// Note: The input biases should be zero.
		INPUT_LAYER.biases.fill(0);
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < INPUT_SIZE; j++) {
				INPUT_LAYER.weights[i][j] = random_num(parameter_min_val, parameter_max_val);
			}
		}

		// Step 3. Initialize weights and biases of first hidden layer.
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				FIRST_HIDDEN.weights[j][i] = random_num(parameter_min_val, parameter_max_val);
			}

			FIRST_HIDDEN.biases[i] = random_num(parameter_min_val, parameter_max_val);
		}

		// Step 4. Initialize weights and biases in second and third layer.
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			for (int j = 0; j < HIDDEN_STD_SIZE; j++) {
				SECOND_HIDDEN.weights[j][i] = random_num(parameter_min_val, parameter_max_val);
			}
			SECOND_HIDDEN.biases[i] = random_num(parameter_min_val, parameter_max_val);

			THIRD_HIDDEN.weights[0][i] = random_num(parameter_min_val, parameter_max_val);
			THIRD_HIDDEN.biases[i] = random_num(parameter_min_val, parameter_max_val);
		}

		// Step 5. Make the output bias zero.
		OUTPUT_LAYER.biases[0] = 0;
	}


	/*
	
	Optimizer. This is the function responsible for delivering work to the threads and updating the parameters. Additionally it should handle I/O.
	
	*/
	void Trainer::run(std::string existing_network) {
		assert(batch_size > 0 && batch_size <= training_data->size());

		// Step 1. Since the training data is already loaded in the constructor, we can immediately move on to setting up the network.
		//	This can either be an existing one which we wish to train even further, or a new randomly initialized one.
		if (existing_network != "") {
			if (existing_network.find(".csv") != std::string::npos) {
				load_net<LNN::CSV>(existing_network);
			}
			else {
				load_net<LNN::BIN>(existing_network);
			}
		}
		else {
			initialize_random();
		}

		// Step 2. Output starting info to the user, set up some data containers and loop through the epochs.
		std::cout << "+----------------------------------------------------------------+\n"
			<< "|                     Loki NN tuning session                     |\n"
			<< "+---------------------+------------------------------------------+\n"
			<< "| Size of dataset			| " << training_data->size() << "\n"
			<< "| Epochs					| " << epochs << "\n"
			<< "| Batch size				| " << batch_size << "\n"
			<< "| Loss function				| " << ((loss_function == LOSS_F::AAE) ? "Average absolute error" : "Mean squared error") << "\n"
			<< "| Initial learning rate		| " << initial_learning_rate << "\n"
			<< "| Learning decay rate		| " << learning_rate_decay << "\n"
			<< "+---------------------+------------------------------------------+" << std::endl;

		// Used to hold the error values for each position in a batch
		std::vector<double> outputs;
		std::vector<double> expected;

		// Each thread gets a vector from these vectors to write errors to.
		std::vector<std::vector<double>> thread_outputs;
		std::vector<std::vector<double>> thread_expected;

		for (int i = 0; i < thread_count; i++) {
			std::vector<double> tmp1, tmp2;
			thread_outputs.push_back(tmp1);
			thread_expected.push_back(tmp2);
		}

		// Vector of positions that the batches will be divided into.
		std::vector<std::vector<TrainingPosition>> thread_positions;

		// Vector for the threads while they're working
		std::vector<std::thread> workers;

		// Step 2A. Determine starting points for each batch.
		std::vector<size_t> batches;

		int batch_count = training_data->size() / batch_size;
		int remainder = training_data->size() % batch_size;

		for (int i = 0; i < batch_count; i++) {
			batches.push_back(i * batch_size);
		}
		if (remainder > 0) {
			batches.push_back(training_data->size() - remainder);
		}
		batches.push_back(training_data->size());

		// Step 2B. Start looping through the epochs.
		for (size_t e = 0; e < epochs; e++) {
			// Compute the learning rate for the epoch.
			learning_rate = initial_learning_rate / (1.0 + learning_rate_decay * double(e));

			// Step 2B.1. Loop through the batches
			for (size_t b = 0; b < batches.size() - 1; b++) {
				// Step 2B.1A. Clear the gradients and data vectors
				main_thread_data->clear_gradients();
				outputs.clear();
				expected.clear();
				thread_positions.clear();

				// Step 2B.1B. Subdivide the batch into sub-batches that the threads will use.
				subdivide_batch(thread_positions, batches, b);
				assert(thread_positions.size() == thread_count);

				// Step 2B.1C. Start up the threads and wait for them to complete their work.
				workers.clear();
				for (int t = 0; t < thread_count; t++) {
					workers.push_back(std::thread(&Trainer::run_thread, this,
																		std::ref(thread_positions[t]),
																		std::ref(thread_outputs[t]),
																		std::ref(thread_expected[t]),
																		t
																		));
				}

				for (int t = 0; t < thread_count; t++) {
					workers[t].join();
				}

				// Step 2B.1D. Compute the average gradients and update the weights
				compute_average_gradients();
				update_parameters(e + 1);

				// Step 2B.1E. Now compute the error of the batch
				outputs = combine_vectors<double>(thread_outputs);
				expected = combine_vectors<double>(thread_expected);
				assert(outputs.size() == expected.size());

				int width = 30;
				int left_padding = (double(b) / double(batches.size())) * width;
				int right_padding = width - left_padding;

				double mse_error = compute_loss<LOSS_F::MSE>(outputs, expected);
				double aae_error = compute_loss<LOSS_F::AAE>(outputs, expected);

				std::string progress(left_padding, '=');
				std::cout << "[" << b + 1 << "/" << batches.size() << "][" << progress << ">" << std::string(right_padding, ' ') << "] "
					<< "MSE:	" << mse_error << "	AAE:	" << aae_error << std::endl;

			}
		}

		// Step 3. Save the network
		if (save_format == LNN::BIN) {
			save_model<LNN::BIN>();
		}
		else {
			save_model<LNN::CSV>();
		}
	}


	/*
	Below are non-functional helper functions.
	*/



	// Batch division. This method divides a batch into work for the different threads
	void Trainer::subdivide_batch(std::vector<std::vector<TrainingPosition>>& sub_batches, const std::vector<size_t>& batches, size_t current) {

		assert(current < batches.size() - 1);
		sub_batches.clear();

		// Determine the starting and ending points in training_data.
		size_t start = batches[current];
		size_t end = batches[current + 1];

		int remainder = (end - start) % thread_count;
		int sub_batch_size = (end - start) / thread_count;

		for (int i = 0; i < thread_count; i++) {
			std::vector<TrainingPosition> sub_batch;

			for (int n = i * sub_batch_size; n < (i + 1) * sub_batch_size; n++) {
				assert(start + n < training_data->size());

				sub_batch.push_back((*training_data)[start + n]);
			}
			sub_batches.push_back(sub_batch);
		}

		// If there is a remainder, add this to the last thread
		if (remainder > 0) {
			for (int n = end - 1; n > end - 1 - remainder; n--) {
				assert(n < training_data->size());
				sub_batches[sub_batches.size() - 1].push_back((*training_data)[n]);
			}
		}
	}



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



	/*
	
	The below method is responsible for setting up a Trainer instance from a command given to Loki.
		Command format: "learn ...":
		Obligatory hyperparameters (in this order):
			- dataset: string
				The csv file containing all the training data.
			- epoch: int [1;+∞]
				Amount of iterations to run the optimization algorithm for.
			- batchsize: int [1;size of dataset]
				The amount of datapoints in a single gradient estimation.
			- loss: string, mse or aae.
				The loss function to use. Either mean squared error or absolute average. Note: This should be rather easy to expand upon.
			- threads: int [1;+∞]. Note: A lot of threads will take up a big portion of memory, so one is adviced to be conservative with this number.
				Threads to use.
		Optional hyperparameters:
			- eta: float [> 0.0], default = 0.01
				initial learning rate
			- eta_decay: float [>= 0.0], default = 0.0001
				Decay of the learning rate after each iteration (for example, eta_decay 0.5 will halve eta each iteration).
				Helpful to avoid passing over a minimum.
			- min_param: float [-∞;+∞], default = -2.0
				Minimum value of parameters if a new net should be trained (randomly initialized).
				Note: If a min_param is passed, a max_param should also be.
			- max_param: float [> min_param], default = 2.0
				Maximum value of parameters if a new net should be trained (randomly initialized).
				Note: If a max_param is passed, a min_param should also be.
			- format: CSV or BIN, default = BIN.
				The format, that the network should be saved as. Either a .csv file or a .lnn binary file.
			- output: string, default = "LokiNet-<Date and time>.lnn/.csv"
				The output file that the saved network should be saved to.
				Note: This needs to match the output format if one is given.
			- net: string, default = ""
				An existing network to train further. If this isn't passed, the algorithm will randomly initialize a new network and train that.
		Example of command:
			learn dataset C:\\Users\\username\\trainingset.csv epoch 1000 batchsize 14500 loss mse threads 4 eta 0.001 eta_decay 0.1 output C:\\Users\\username\\output.lnn

			This will train a new network for 1000 epochs, with a batchsize and loss function of 14500 and mean squared error respectively. It will use 4 threads, a learning
			rate of 0.001, a learning decay of 0.1, and save the network to C:\\Users\\username\\output.lnn.
			The training set used will be C:\\Users\\username\\trainingset.csv
	*/
	bool parse_learn(std::string cmd) {
		// Step 1. Initialize all hyperparameters.
		std::string dataset = "", loss_str = "";
		size_t epoch = -1, batch_size = -1;
		LOSS_F loss = LOSS_F::MSE;
		size_t threads = -1;

		double eta = LEARNING_RATE_DEFAULT, eta_decay = LEARNING_DECAY_DELAULT, min_param = -DEFAULT_WEIGHT_BOUND, max_param = DEFAULT_WEIGHT_BOUND;
		LNN::LNN_FileType format = LNN::BIN;
		std::string output_file = "", existing_net = "";
		
		// Step 2. Parse all the obligatory parameters, and return if some of them are missing.
		int index = 0;

		try {

			// Step 2A. Dataset.
			index = cmd.find("dataset");

			if (index != std::string::npos) {
				dataset = cmd.substr(index + 8);

				// If there are more parameters after this, we need to split dataset and use the part before the next space
				// Note if the path has been given inside quotation marks, use this instead of space.
				size_t last_index = std::string::npos;
				if (dataset[0] == '"') {
					last_index = (dataset.substr(1)).find_first_of('"');
					dataset = dataset.substr(1, last_index);
				}
				else {
					last_index = dataset.find_first_of(" ");
					dataset = dataset.substr(0, last_index);
				}
			}
			else { throw("A path to the data-file must be specified."); }

			// Step 2B. Number of epochs
			index = cmd.find("epoch");

			if (index != std::string::npos) {

			}
			else { throw("An epoch count must be specified."); }

			// Step 2C. Batch size
			index = cmd.find("batchsize");

			if (index != std::string::npos) {

			}
			else { throw("A batch size must be specified."); }

			// Step 2D. Loss function
			index = cmd.find("loss");

			if (index != std::string::npos) {

			}
			else { throw("A loss function must be specified."); }

			// Step 2E. Thread count
			index = cmd.find("threads");

			if (index != std::string::npos) {

			}
			else { throw("A thread count must be specified."); }
		}
		catch (const char* msg) {
			std::cout << "[!] Training session setup encountered an error: " << msg << std::endl;
			return false;
		}


		return true;
	}
}