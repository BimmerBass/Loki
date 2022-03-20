/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "texel.h"



namespace Texel {

	/*
	Load an EPD file containing the FENs and the game results.
	*/
	
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

	/*
		thread_batch is run by each individual thread. It computes the squared differences between a position's eval and the game result, for the particular partition
		the main thread has alotted to it.
	*/

	void thread_batch(tuning_positions* EPDS, double k, double* sum) {

		GameState_t* pos = new GameState_t;
		Eval::Evaluate eval;
		int value = 0;

		for (int i = 0; i < EPDS->size(); i++) {

			// Step 1. Parse the fen.
			pos->parseFen((*EPDS)[i].fen);

			// Step 2. Evaluate and make the result relative to white
			value = eval.score(pos, false);
			value *= (pos->side_to_move == WHITE) ? 1 : -1;

			// Step 3. Square the difference between the game result and the eval, and add this to the sum.
			*sum += std::pow(((*EPDS)[i].game_result - sigmoid(value, k)), 2.0);
		}

		delete pos;
	}

	/*
		This function runs eval::evaluate on all positions in the EPD provided and compares the sigmoid of these with the game restult. From this, it
		computes the error as the sum of the squared differences, divided by the amount of positions.
	*/

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

	/*
		This function changes the parameters in the evaluation function and computes the new error based on the EPD-file provided and the value of k.
	*/

	double changed_error(Parameters p, std::vector<Score> new_values, tuning_positions* EPDS, double k) {

		assert(p.size() == new_values.size());

		// Step 1. Change the values in p with the ones in new_values
		for (int i = 0; i < new_values.size(); i++) {
			p[i].variable->mg = new_values[i].mg;
			p[i].variable->eg = new_values[i].eg;
		}

		// Step 2. Compute the new error and return
		return mean_squared_error(EPDS, k);
	}



	/*
		This function is called at the start of the tuning run. It will find the value k such that the sigmoid function gets mapped to a win probability based on
		the existing evaluation function. After that, k will not be changed further.
	*/

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


	/*
		The main tuning function of the framework. This is responsible for handling the changing of the parameters and the updating of these.
		In the end, it calls a function to write the results of the tuning session to a .csv file.
	*/

	void Tune(Parameters tuning_vars, std::string epd_file, int iterations) {


		// Step 1. Load the EPD file and create a new vector of tuning data.
		tuning_positions* EPDS = load_epd(epd_file);
		TexelStats::TuningData data;


		// Step 2. Set up the vector of parameter values, we call it theta here.
		std::vector<Score> theta;

		for (int p = 0; p < tuning_vars.size(); p++) {
			theta.push_back(Score(tuning_vars[p].variable->mg, tuning_vars[p].variable->eg));
		}



		// Step 3. Compute the optimal value of k
		double k = optimal_k(EPDS);



		// Step 4. Calculate SPSA constants

		double BIG_A = 0.1 * double(iterations);

		for (int p = 0; p < tuning_vars.size(); p++) {

			tuning_vars[p].c.mg = tuning_vars[p].C_END.mg * std::pow(double(iterations), gamma);
			tuning_vars[p].c.eg = tuning_vars[p].C_END.eg * std::pow(double(iterations), gamma);



			double a_end_mg = tuning_vars[p].R_END.mg * std::pow(tuning_vars[p].c.mg, 2.0);
			double a_end_eg = tuning_vars[p].R_END.eg * std::pow(tuning_vars[p].c.eg, 2.0);

			tuning_vars[p].a.mg = a_end_mg * std::pow(BIG_A + double(iterations), alpha);
			tuning_vars[p].a.eg = a_end_eg * std::pow(BIG_A + double(iterations), alpha);
		}

		//double c = C_END * std::pow(double(iterations), gamma);
		//double a_end = R_END * std::pow(c, 2.0);
		//double a = a_end * std::pow(BIG_A + double(iterations), alpha);



		// Step 5. Run the tuning with the given number of iterations
		std::cout << "\n[*] Initialization complete. Starting AdamSPSA tuning session for " << iterations << " iterations." << std::endl;

		std::vector<Score> theta_plus;
		std::vector<Score> theta_minus;

		std::vector<Score> delta; // Initialize perturbation vector.

		double g_hat_mg = 0.0;
		double g_hat_eg = 0.0;

		//double an = 0.0;
		//double cn = 0.0;


		/*
		Zero-initialization of step-size and perturbation vectors
		*/
		std::vector<Value> an;
		std::vector<Value> cn;

		for (int p = 0; p < tuning_vars.size(); p++) {
			an.push_back(Value(0.0, 0.0));
			cn.push_back(Value(0.0, 0.0));
		}

		for (int n = 0; n < iterations; n++) {

			// Step 5A. Compute the current error. This is only used for outputting the progress.
			double error = changed_error(tuning_vars, theta, EPDS, k);

			//data.push_back(TexelStats::DataPoint(n + 1, error, Score(g_hat_mg, g_hat_eg), Score(abs(an * g_hat_mg), abs(an * g_hat_eg)), theta));
			data.push_back(TexelStats::DataPoint(n + 1, error, Score(g_hat_mg, g_hat_eg), Score(0, 0), theta));

			// Step 5B. Calculate iteration dependent constants
			//an = a / (std::pow(BIG_A + double(n) + 1.0, alpha));
			//cn = c / (std::pow(double(n) + 1, gamma));
			for (int p = 0; p < tuning_vars.size(); p++) {
				an[p].mg = tuning_vars[p].a.mg / (std::pow(BIG_A + double(n) + 1.0, alpha));
				an[p].eg = tuning_vars[p].a.eg / (std::pow(BIG_A + double(n) + 1.0, alpha));

				cn[p].mg = tuning_vars[p].c.mg / (std::pow(double(n) + 1.0, gamma));
				cn[p].eg = tuning_vars[p].c.eg / (std::pow(double(n) + 1.0, gamma));
			}


			std::cout << "[+] Iteration " << (n + 1) << std::endl; //  << ": an = " << an << ", cn = " << cn << std::endl;
			std::cout << "[+] Initial error: " << error << std::endl;

			// Clear theta_plus, theta_minus and delta.
			theta_plus.clear();
			theta_minus.clear();
			delta.clear();


			// Step 5C. Determine delta and thus theta_plus and theta_minus
			for (int p = 0; p < tuning_vars.size(); p++) {
				int d_mg = randemacher();
				int d_eg = randemacher();

				delta.push_back(Score(d_mg, d_eg)); // Add the scores to the delta vector.

				// Step 5C.1. Compute theta_plus and theta_minus from these values
				//theta_plus.push_back(Score(std::round(theta[p].mg + double(d_mg) * cn), std::round(theta[p].eg + double(d_eg) * cn)));
				//theta_minus.push_back(Score(std::round(theta[p].mg - double(d_mg) * cn), std::round(theta[p].eg - double(d_eg) * cn)));
				theta_plus.push_back(Score(std::round(theta[p].mg + double(d_mg) * cn[p].mg), std::round(theta[p].eg + double(d_eg) * cn[p].eg)));
				theta_minus.push_back(Score(std::round(theta[p].mg - double(d_mg) * cn[p].mg), std::round(theta[p].eg - double(d_eg) * cn[p].eg)));
			}

			// Step 5D. Compute the error of theta_plus and theta_minus respectively.
			double theta_plus_error = double(EPDS->size()) * changed_error(tuning_vars, theta_plus, EPDS, k);
			double theta_minus_error = double(EPDS->size()) * changed_error(tuning_vars, theta_minus, EPDS, k);

			std::cout << "Theta plus error: " << theta_plus_error << ", theta minus error: " << theta_minus_error << std::endl;



			// Step 5E. Compute the gradient for each variable and adjust accordingly.
			for (int p = 0; p < tuning_vars.size(); p++) {

				//g_hat_mg = (theta_plus_error - theta_minus_error) / (2.0 * cn * double(delta[p].mg));
				//g_hat_eg = (theta_plus_error - theta_minus_error) / (2.0 * cn * double(delta[p].eg));
				g_hat_mg = (theta_plus_error - theta_minus_error) / (2.0 * cn[p].mg * double(delta[p].mg));
				g_hat_eg = (theta_plus_error - theta_minus_error) / (2.0 * cn[p].eg * double(delta[p].eg));

				// Now adjust the theta values based on the gradient.
				//theta[p].mg -= std::round(an * g_hat_mg);
				//theta[p].eg -= std::round(an * g_hat_eg);

				theta[p].mg -= std::round(an[p].mg * g_hat_mg);
				theta[p].eg -= std::round(an[p].eg * g_hat_eg);

				// Lastly, we'll have to make sure that we don't go out of the designated bounds
				theta[p].mg = std::max(tuning_vars[p].min_value.mg, std::min(tuning_vars[p].max_value.mg, theta[p].mg));
				theta[p].eg = std::max(tuning_vars[p].min_value.eg, std::min(tuning_vars[p].max_value.eg, theta[p].eg));
			}

			// Step 5F. Display the current values.
			std::cout << "Values after " << (n + 1) << " iterations: " << std::endl;
			
			for (int p = 0; p < tuning_vars.size(); p++) {
				std::cout << "[" << (p + 1) << "]: mg = " << theta[p].mg << ",	eg = " << theta[p].eg << ",		(Original: [" << tuning_vars[p].original_value.mg 
					<< ", " << tuning_vars[p].original_value.eg << "])" << std::endl;
			}

			std::cout << "\n\n";
		}

		delete EPDS;


		// Step 6. Write the results to a .csv file.
		TexelStats::write_csv(data);
	}





	/*
		This method writes all tuning results (error, resulting values, gradients etc..) to a .csv file named "LokiTexel-<date and time>.csv"
	*/
	void TexelStats::write_csv(TexelStats::TuningData data) {

		// Step 1. Create the filename
		std::string date_time = getDateTime();
		std::string filename = "LokiTexel-" + date_time + ".csv";


		// Step 2. Create the file.
		std::ofstream outFile(filename);


		// Step 3. Write all the data.


		// Step 3A. Write iterations.
		outFile << "Iteration; ";

		outFile << data[0].iteration;

		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].iteration;
		}
		outFile << "\n";


		// Step 3B. Write error values
		outFile << "Error; ";
		outFile << data[0].error;

		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].error;
		}
		outFile << "\n";

		// Step 3C. Write the gradient

		// Step 3C.1. Write the middlegame gradient
		outFile << "Gradient MG;";
		outFile << data[0].gradient.mg;

		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].gradient.mg;
		}
		outFile << "\n";

		// Step 3C.1. Write the endgame gradient
		outFile << "Gradient EG;";
		outFile << data[0].gradient.eg;

		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].gradient.eg;
		}
		outFile << "\n";



		// Step 3D. Write the variable's values.

		for (int v = 0; v < data[0].values.size(); v++) {

			// Step 3D.1. Write middlegame data first.
			outFile << "Variable " << (v + 1) << " MG;";
			outFile << data[0].values[v].mg;

			for (int i = 0; i < data.size(); i++) {
				outFile << ";" << data[i].values[v].mg;
			}
			outFile << "\n";


			// Step 3D.1. Write endgame data first.
			outFile << "Variable " << (v + 1) << " EG;";
			outFile << data[0].values[v].eg;

			for (int i = 1; i < data.size(); i++) {
				outFile << ";" << data[i].values[v].eg;
			}
			outFile << "\n";

		}

	}

}