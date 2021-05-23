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
		std::string line = "";

		if (!data_file.is_open()) {
			std::cout << "[!] Error loading the data. Please make sure the file exists and that the right path is given" << std::endl;
			abort();
		}

		// Step 2. Now go through all lines in the file.
		while (std::getline(data_file, line)) {

		}
	}
}