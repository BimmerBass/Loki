#ifndef BENCH_H
#define BENCH_H
#include "search.h"

#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

// Function to get the date and time to write to the filename
inline std::string getDateTime()
{
	auto time = std::time(nullptr);
	std::stringstream ss;

#if (defined(_WIN32) || defined(_WIN64))
	tm ltm;
	localtime_s(&ltm, &time);
	ss << std::put_time(&ltm, "%F_%T"); // ISO 8601 without timezone information.
#else
	ss << std::put_time(std::localtime(&time), "%F_%T");
#endif
	auto s = ss.str();
	std::replace(s.begin(), s.end(), ':', '-');
	return s;
}



namespace Bench {

	namespace SMP {

		struct DataPoint {
			int threads = 0;		// Amount of threads used
			int depth = 0;			// Depth searched to
			double ttd_avg = 0;		// Average time-to-depth
			double ttd_dev = 0;		// Standard deviation of time-to-depth measurements
		};

		extern DataPoint results[THREADS_MAX_NUM][MAXDEPTH];

		// Measures the time-to-depth stats of Lazy SMP and writes results to a .csv file.
		void ttd_stats(int threads, int depth_min, int depth_max, int iterations, std::vector<std::string> fen_list);
	}


}








#endif