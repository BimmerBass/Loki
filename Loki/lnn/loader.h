#ifndef LOADER_H
#define LOADER_H
#include <iostream>
#include <array>
#include <assert.h>
#include <memory>

#include "types/architecture.h"
/*

This file is responsible for loading the network files. The reason for not doing this in the net class is twofold:
	1) The trainer uses floating point numbers and saves the files accordingly. For quantization to work, we need to firstly read floats and then do the number
		transformation.
	2) To have all net-file handling in one place :))

*/



namespace NetLoading {
	constexpr size_t PARAMETER_COUNT = INPUT_SIZE * FIRST_HIDDEN_SIZE
		+ FIRST_HIDDEN_SIZE + FIRST_HIDDEN_SIZE * HIDDEN_STD_SIZE
		+ HIDDEN_STD_SIZE + HIDDEN_STD_SIZE * HIDDEN_STD_SIZE
		+ HIDDEN_STD_SIZE + HIDDEN_STD_SIZE;

	// This structure holds all data from the file.
	struct WeightData {
		// Input weights
		std::array<std::array<float, INPUT_SIZE>, FIRST_HIDDEN_SIZE> INPUT_WEIGHTS;

		// First hidden layer
		std::array<float, FIRST_HIDDEN_SIZE> FH_BIAS;
		std::array<std::array<float, FIRST_HIDDEN_SIZE>, HIDDEN_STD_SIZE> FH_WEIGHTS;

		// Second hidden layer.
		std::array<float, HIDDEN_STD_SIZE> SH_BIAS;
		std::array<std::array<float, HIDDEN_STD_SIZE>, HIDDEN_STD_SIZE> SH_WEIGHTS;

		// Third (last) hidden layer.
		std::array<float, HIDDEN_STD_SIZE> TH_BIAS;
		std::array<float, HIDDEN_STD_SIZE> TH_WEIGHTS;

		// Flag for telling the function caller if the file were read properly.
		bool success = false;
	};

	// Function for loading a net from a file.
	std::unique_ptr<WeightData> load_from_file(std::string path);

	// Function for loading an embedded net
	std::unique_ptr<WeightData> load_embedded();

}



#endif