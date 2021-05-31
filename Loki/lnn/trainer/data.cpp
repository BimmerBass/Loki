#include "data.h"



/*

Constructor. This should be responsible for opening the file, setting the parameters and reporting back errors.

*/

DataLoader::DataLoader(std::string filepath, size_t _fetch_count) : incremental_count(_fetch_count) {


}