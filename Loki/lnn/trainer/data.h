#ifndef DATA_H
#define DATA_H
#include <string>
#include <vector>
#include <iostream>

#include "../types/architecture.h"



namespace Data {

	// Constants
	constexpr size_t DEFAULT_BATCH_COUNT = 2000;

	
	// Structure for holding a single training example
	struct DataEntry {
		int8_t network_input[INPUT_SIZE] = { 0 };
		int score = 0;

		void clear(){
			memset(network_input, int8_t(0), sizeof(int8_t) * INPUT_SIZE);
			score = 0;
		}
	};


	/*
	Used for saving the data during generation.
	*/
	class DataWriter {
	public:
		DataWriter(std::string filepath);
		~DataWriter();

		void save_data(const std::vector<DataEntry>& data);
	private:
		// File stream handling
		FILE* file = nullptr;
		
	};


	/*
	Used for loading a file during training.
	*/
	class DataLoader {
	public:
		DataLoader(std::string filepath, size_t _bs, size_t bfc = DEFAULT_BATCH_COUNT);
		~DataLoader();

		bool fetch_data(std::vector<DataEntry>& data);

		size_t size() const { return entry_count; }
	private:
		// File stream handling
		FILE* file = nullptr;

		// Data.
		size_t entry_count;
		size_t current_entry;

		// Fetching parameters
		const size_t batch_size;
		const size_t batch_fetch_count;
		const size_t entry_fetch_count;
	};
}



#endif