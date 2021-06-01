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
	fwrite(data.data(), sizeof(DataEntry), data.size(), file);
}