#include "bench.h"

namespace Bench {

	namespace SMP {

		DataPoint results[THREADS_MAX_NUM][MAXDEPTH] = {};


		void ttd_stats(int threads, int depth_min, int depth_max, int iterations, std::vector<std::string> fen_list) {

			// For the sake of security, we'll clear the results array
			for (int t = 0; t < THREADS_MAX_NUM; t++) {
				for (int d = 0; d < MAXDEPTH; d++) {
					results[t][d].threads = 0;
					results[t][d].depth = 0;
					results[t][d].ttd_avg = 0;
					results[t][d].ttd_dev = 0;
				}
			}


			for (int thread_num = 1; thread_num <= threads; thread_num++) {

				for (int depth = depth_min; depth <= depth_max; depth++) {
					double average = 0.0;
					double variance = 0.0;
					double deviation = 0.0;

					std::vector<double> time_measurements;

					// Compute average time.
					for (int i = 0; i < iterations; i++) {
						double iteration_average = 0.0;

						GameState_t* pos = new GameState_t;
						SearchInfo_t* info = new SearchInfo_t;

						info->timeset = false;
						info->starttime = getTimeMs();
						info->depth = depth;
						
						// Compute the average time-to-depth for all the positions in the fen_list
						for (int p = 0; p < fen_list.size(); p++) {
							pos->parseFen(fen_list[p]);
							std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

							Search::runSearch(pos, info, threads);

							std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();

							auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_time).time_since_epoch().count();
							auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();
							//iteration_average += ttd(thread_num, depth, fen_list[p]);
							iteration_average += (end - start);
						}
						iteration_average /= fen_list.size();

						time_measurements.push_back(iteration_average);
						
						average += time_measurements[i];

						delete pos;
						delete info;
						
						//tt->clear_table();
					}
					average /= iterations;


					// Compute the standard deviation of the measurements.
					for (int i = 0; i < iterations; i++) {
						variance += std::pow(average - time_measurements[i], 2);
					}
					variance /= (iterations - 1);

					deviation = std::sqrt(variance);


					// Store the results
					results[thread_num][depth].threads = thread_num;
					results[thread_num][depth].depth = depth;
					results[thread_num][depth].ttd_avg = average;
					results[thread_num][depth].ttd_dev = deviation;
				}
			}


			/* Write results to a .csv file */

			std::string filename = "benchmark-";
			filename += getDateTime();
			filename += ".csv";


			std::ofstream outFile(filename);

			// Firstly, write all average times

			outFile << "Average times\n";

			for (int i = 0; i < threads; i++) {
				if (i == 0) {
					outFile << "1 thread;";
				}
				else {
					outFile << (i + 1) << " threads;";
				}
			}

			outFile << "\n";

			for (int d = depth_min; d <= depth_max; d++) {

				for (int t = 1; t <= threads; t++) {
					outFile << results[t][d].ttd_avg << ";";
				}

				outFile << "\n";
			}

			// Now we can create a new table with all the standard deviations.

			outFile << "\nAverage time deviations\n";

			for (int i = 0; i < threads; i++) {
				if (i == 0) {
					outFile << "1 thread;";
				}
				else {
					outFile << (i + 1) << " threads;";
				}
			}

			outFile << "\n";

			for (int d = depth_min; d <= depth_max; d++) {

				for (int t = 1; t <= threads; t++) {
					outFile << results[t][d].ttd_dev << ";";
				}

				outFile << "\n";
			}

		}




	}
}