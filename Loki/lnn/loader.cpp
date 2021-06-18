#include "loader.h"



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



namespace NetLoading {

	/*
	
	Load a net from a binary (.lnn) file.
	
	*/
	std::unique_ptr<WeightData> load_from_file(std::string path) {
		// Step 1. Allocate a new WeightData structure.
		// Note: If we return due to an error, the we need to have this specified.
		std::unique_ptr<WeightData> data(new WeightData);
		data->success = false;

		FILE* pFile = nullptr;

		try {
			// Step 2. Open the file and make sure that 1) It exists, and 2) It has the right size/amount of parameters.
#if (defined(_WIN32) || defined(_WIN64))
			fopen_s(&pFile, path.c_str(), "rb");
#else
			pFile = fopen(path.c_str(), "rb");
#endif
			// File couldn't be opened.
			if (pFile == nullptr) {
				throw("info string error failed to open file. Please specify a path to a valid LNN file.");
			}

			fseek(pFile, 0, SEEK_END);
			size_t pos = ftell(pFile);
			rewind(pFile);
			size_t num_params = pos / sizeof(float);

			if (num_params != PARAMETER_COUNT) {
				throw("info string error the specified file does not contain the right amount of data.");
			}

			/* Step 2. We can now go on to reading the file.It is formatted in the following way:
				- Input weights
				- First hidden biases
				- First hidden weights
				- Second hidden biases
				- Second hidden weights
				- Third hidden biases
				- Third hidden weights
			*/
			for (size_t i = 0; i < FIRST_HIDDEN_SIZE; i++) {
				fread(data->INPUT_WEIGHTS[i].data(), sizeof(float), INPUT_SIZE, pFile);
			}

			fread(data->FH_BIAS.data(), sizeof(float), FIRST_HIDDEN_SIZE, pFile);
			for (size_t i = 0; i < HIDDEN_STD_SIZE; i++) {
				fread(data->FH_WEIGHTS[i].data(), sizeof(float), FIRST_HIDDEN_SIZE, pFile);
			}

			fread(data->SH_BIAS.data(), sizeof(float), HIDDEN_STD_SIZE, pFile);
			for (size_t i = 0; i < HIDDEN_STD_SIZE; i++) {
				fread(data->SH_WEIGHTS[i].data(), sizeof(float), HIDDEN_STD_SIZE, pFile);
			}

			fread(data->TH_BIAS.data(), sizeof(float), HIDDEN_STD_SIZE, pFile);
			fread(data->TH_WEIGHTS.data(), sizeof(float), HIDDEN_STD_SIZE, pFile);

			// Step 3. If all data was written properly, toggle the success flag.
			data->success = true;

		}
		catch (const char* msg) {
			std::cout << msg << std::endl;
		}

		if (pFile != nullptr) { fclose(pFile); }

		return data;
	}



}