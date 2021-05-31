#ifndef DATA_H
#define DATA_H
#include <string>
#include <vector>

#include "../types/architecture.h"


constexpr size_t DEFAULT_FETCH_COUNT = 2000000;

// A structure for holding a training position
struct TrainingPosition {
	int8_t network_inputs[INPUT_SIZE] = { 0 };
	int score;

	void set(int val) {
		memset(network_inputs, int8_t(0), sizeof(int8_t) * INPUT_SIZE);
		score = val;
	}

	TrainingPosition() {
		memset(network_inputs, int8_t(0), sizeof(int8_t) * INPUT_SIZE);
		score = 0;
	}
	TrainingPosition(const TrainingPosition& tp) {
		memcpy(network_inputs, tp.network_inputs, sizeof(int8_t) * INPUT_SIZE);
		score = tp.score;
	}
};


/*

The DataLoader class is responsible for loading the data gradually as needed.
This class is important since the datasets usually are so big, that all of it can't fit into memory.

*/
class DataLoader {
public:
	DataLoader(std::string filepath, size_t _fetch_count = DEFAULT_FETCH_COUNT);
	~DataLoader();

	void fetch_more_data(std::vector<TrainingPosition>& data);

private:
	FILE* dataFile = nullptr;
	size_t current_entry = 0;
	size_t entry_count = 0;

	const size_t incremental_count;
};







#endif