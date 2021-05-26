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
}