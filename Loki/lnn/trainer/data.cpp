#include "data.h"




/*

Data writer class

*/
Data::DataWriter::DataWriter(std::string filepath) {
	// Step 1. Attempt to open the file in binary writing mode.
#ifdef _MSC_VER
	fopen_s(&file, filepath.c_str(), "wb");
#else
	file = fopen(filepath.c_str(), "wb");
#endif

	// Step 1A. Throw an error if the file couldn't be opened
	try {
		if (file == nullptr) { throw("The output file couldn't be opened."); }
	}
	catch (const char* msg) {
		std::cout << "[!] Error initializing DataWriter object: " << msg << std::endl;
		exit(EXIT_FAILURE);
	}
}

Data::DataWriter::~DataWriter() {
	if (file != nullptr) {
		fclose(file);
	}
}


/*

Data writing.
	- Param: vector of datapoints to write.
*/
void Data::DataWriter::save_data(const std::vector<DataEntry>& data) {
	// Step 1. Write all data from the vector.
	//fwrite(data.data(), sizeof(DataEntry), data.size(), file);
	for (int i = 0; i < data.size(); i++) {
		fwrite(data[i].network_input, sizeof(int8_t), INPUT_SIZE, file);
		fwrite(&data[i].score, sizeof(int), 1, file);
	}
}




/*

Data loader class.

*/
Data::DataLoader::DataLoader(std::string filepath, size_t _bs, size_t bfc) : batch_size(_bs), batch_fetch_count(bfc), entry_fetch_count(_bs* bfc) {
	// Step 1. Make sure all inputs are properly configured.
	try {
		if (filepath == "") { throw("No file path specified"); }
		if (_bs < 1) { throw("Incorrect batch size specified. This must be a positive number."); }
		if (bfc < 1) { throw("Incorrect batch fetch count specified. This must be a positive number."); }
	}
	catch (const char* msg) {
		std::cout << "[!] An error was encountered in the DataLoader constructor: " << msg << std::endl;
		exit(EXIT_FAILURE);
	}

	// Step 2. Attempt to open the file and throw an error if this fails.
#ifdef _MSC_VER
	fopen_s(&file, filepath.c_str(), "rb");
#else
	file = fopen(filepath.c_str(), "rb");
#endif

	try {
		if (file == nullptr) { throw("Couldn't open data file"); }
	}
	catch (const char* msg) {
		std::cout << "[!] Error while opening file: " << msg << std::endl;
		exit(EXIT_FAILURE);
	}

	// Step 3. Now read the amount of training data entries in the file.
	current_entry = 0;
	fseek(file, 0, SEEK_END);
	size_t bytes = ftell(file);
	//entry_count = bytes / sizeof(DataEntry);
	entry_count = bytes / (sizeof(int8_t) * INPUT_SIZE + sizeof(int));

	try {
		if (entry_count < 1) { throw("An empty file was loaded."); }
	}
	catch (const char* msg) {
		std::cout << "[!] Error encountered reading file: " << msg << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Loaded file with " << entry_count << " entries" << std::endl;

	// Step 4. Go back to the beginning of the file.
	rewind(file);
}

Data::DataLoader::~DataLoader() {
	if (file != nullptr) {
		fclose(file);
	}
}


/*

Fetch a new amount of batches.

*/
bool Data::DataLoader::fetch_data(std::vector<DataEntry>& data) {
	// Step 1. Clear the data and reserve the - estimated - required memory.
	data.clear();
	data.reserve(entry_fetch_count);

	// Step 2. Now fetch the desired amount of batches, and break if we reach EOF
	bool reached_end = false;

	for (int f = 0; f < batch_fetch_count; f++) {

		// Step 2A. Now load entries until we either reach EOF or have filled an entire batch
		for (int i = 0; i < batch_size; i++) {
			DataEntry entry;

			//size_t read = fread(&entry, sizeof(DataEntry), 1, file);
			//if (read != 1) { std::cout << "Error reading file. Quitting." << std::endl; exit(EXIT_FAILURE); }
			fread(entry.network_input, sizeof(int8_t), INPUT_SIZE, file);
			fread(&entry.score, sizeof(int), 1, file);

			current_entry++;

			data.push_back(entry);

			// Step 2A.1. Handle EOF
			if (current_entry > entry_count) {
				reached_end = true;
				break;
			}
		}

		// Step 2B. Handle EOF.
		if (reached_end) {
			break;
		}
	}

	// Step 3. If we reached EOF, go back to the start of the file.
	if (reached_end) {
		current_entry = 0;
		rewind(file);
	}

	// Step 4. Return true if there is more data left, otherwise return false.
	// This will signal to start a new epoch.
	return !reached_end;
}