#include "texel.h"



namespace Texel {


	// Function responsible for extracting the FEN's and game results from an EPD file.
	tuning_positions* load_epd(std::string path) {
		tuning_positions* positions = new tuning_positions();

		std::ifstream epd_file(path);


		// The data we extract from each line of the EPD.
		std::string epd;
		std::string fen;
		double result;

		int count = 0;


		while (std::getline(epd_file, epd)) { // Get another epd.

			fen = "";

			auto fen_end = epd.find_first_of("-");

			if (epd[fen_end + 2] == '-') {
				fen_end = fen_end + 2;
			}

			fen = epd.substr(0, fen_end + 1);

			// Now we need to resolve the game result

			auto result_start = epd.find_first_of('"');

			std::string res = epd.substr(result_start + 1, epd.length() - result_start - 3);

			if (res == "1/2-1/2") {
				result = 0.5;
			}
			else if (res == "1-0") {
				result = 1.0;
			}
			else if (res == "0-1") {
				result = 0.0;
			}
			else { // Throw an error if no result is given.
				assert(false);
			}

			// Push back the new position.
			positions->push_back(texel_position(fen, result));
		}

		return positions;
	}


	void thread_batch(tuning_positions* EPDS, double k, double* sum) {

		GameState_t* pos = new GameState_t;
		int value = 0;

		for (int i = 0; i < EPDS->size(); i++) {

			// Step 1. Parse the fen.
			pos->parseFen((*EPDS)[i].fen);

			// Step 2. Evaluate and make the result relative to white
			value = Eval::evaluate(pos);
			value *= (pos->side_to_move == WHITE) ? 1 : -1;

			// Step 3. Square the difference between the game result and the eval, and add this to the sum.
			*sum += std::pow(((*EPDS)[i].game_result - sigmoid(value, k)), 2.0);
		}

		delete pos;
	}


	double mean_squared_error(tuning_positions* EPDS, double k) {

		double sums[EVAL_THREADS] = { 0 };
		tuning_positions batches[EVAL_THREADS];

		// Step 1. Partition the positions between the threads
		int partition_size = EPDS->size() / EVAL_THREADS;
		int current = 0;
		for (int t = 0; t < EVAL_THREADS; t++) {

			int i = 0;

			while (i <= partition_size && (i + current) < EPDS->size()) {
				batches[t].push_back((*EPDS)[current + i]);

				i++;
			}

			current += i;
		}


		if (EVAL_THREADS > 1) {
			std::vector<std::thread> workers;

			for (int t = 0; t < EVAL_THREADS; t++) {
				workers.push_back(std::thread(thread_batch, &batches[t], k, &sums[t]));
			}

			// Wait for the workers to complete their work, and then join them
			for (int t = 0; t < EVAL_THREADS; t++) {
				workers[t].join();
			}

		}
		else { // Don't start up any workers. Just run the thread_batch function
			thread_batch(&batches[0], k, &sums[0]);
		}


		double avg = 0;

		for (int i = 0; i < EVAL_THREADS; i++) {
			avg += sums[i];
		}

		return (avg / double(EPDS->size()));

	}










	double optimal_k(tuning_positions* EPDS) {
		std::cout << "Finding optimal value of K." << std::endl;
		std::cout << "Initial value: " << k_initial << std::endl;


		double best_k = k_initial;
		double best_error = mean_squared_error(EPDS, best_k);

		std::cout << "Initial error: " << best_error << std::endl;

		
		for (int i = 0; i <= k_precision; i++) {
			std::cout << "Iteration " << (i + 1) << ": " << std::endl;
			
			double unit = std::pow(10.0, -i);
			double range = 10.0 * unit;
			
			double k_max = best_k + range;
			
			for (double k = std::max(best_k - range, 0.0); k <= k_max; k += unit) {

				double error = mean_squared_error(EPDS, k);
			
				std::cout << "Error of K = " << k << ": " << error << std::endl;
			
				if (error < best_error) {
					best_error = error;
					best_k = k;
				}
			
			}
			
			std::cout << "[+] Best K at iteration " << (i + 1) << ": " << best_k << " with error E = " << best_error << std::endl;
		}

		std::cout << "Optimal K = " << best_k << std::endl;

		
		return best_k;
	}


}