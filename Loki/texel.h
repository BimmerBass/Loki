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
#ifndef TEXEL_H
#define TEXEL_H

#include "uci.h" // Include all of the engine

#include <vector>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>

// Amount of concurrent threads to run when computing the eval error.
// NOTE: Multithreaded performance should be measured for the particular PC, since the speed doesn't keep rising with the number of threads.
// On the author's PC the best performance was reached at 8 threads, hence the number below.
constexpr int EVAL_THREADS = 8;

static_assert(EVAL_THREADS >= 1);





// Seed the RNG with the time since epoch.
inline void seed_random() {
	std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
}



// This is the generator of the Bernoulli +/- 1 distribution with p = 50%, used in the tuning
static std::default_random_engine generator;
static std::bernoulli_distribution distribution(0.5);

inline double randemacher() {
	return (distribution(generator)) ? 1 : -1;
}



using namespace PSQT;


namespace Texel {
	/*
	Parameters for the sigmoid constant
	*/
	constexpr double k_initial = 1.0;
	constexpr int k_precision = 4; // Amount of digits to use.


	/*
	SPSA constants
	*/
	constexpr double alpha = 0.602;
	constexpr double gamma = 0.101;

	//constexpr double R_END = 0.003; // 0.002
	//constexpr double C_END = 4.0;

	// The two latter values are to be changed for the particular tuning session. A goal is to decide these automatically for each parameter.


	/*
	The Value struct just holds two doubles representing the middle-game and endgame values
	*/
	struct Value {
		Value(double m, double e) { mg = m; eg = e; }
		Value() { mg = 0.0; eg = 0.0; }
		double mg;
		double eg;
	};



	/*
	This structure holds all the necessary data for the parameter, which is its original value, the address (to change it in eval) and its bounds if specified.
	*/
	struct Parameter {

		Parameter (Score* var, Value _rend = Value(0.002, 0.002), Value _cend = Value(4.0, 4.0), Score max_val = Score(INF, INF), Score min_val = Score(-INF, -INF)) {
			variable = var; // Copy the pointer

			max_value = max_val;
			min_value = min_val;

			original_value = *var;

			C_END = _cend;
			R_END = _rend;
		}

		Score* variable;


		Score max_value;
		Score min_value;


		Score original_value;

		Value C_END;
		Value R_END;
		Value c;
		Value a;
	};

	typedef std::vector<Parameter> Parameters;

	struct texel_position {
		texel_position(std::string _fen, double result) {
			fen = _fen;
			game_result = result;
		}

		std::string fen = "";

		double game_result = 0.0; // Results are respresented by 1: white win, 0.5: draw, 0.0: black win.
	};

	typedef std::vector<texel_position> tuning_positions;



	/*
	The TexelStats namespace has structures that holds all information about the tuning run. It also has a method responsible for writing results to a .csv file.
	*/
	namespace TexelStats {

		struct DataPoint {

			DataPoint(int i, double e, Score g, Score ss, std::vector<Score> theta) {
				iteration = i; error = e; gradient = g; step_size = ss; values = theta;
			}

			int iteration = 0;

			std::vector<Score> values;

			double error;

			Score gradient;
			Score step_size;
		};

		typedef std::vector<DataPoint> TuningData;


		void write_csv(TuningData data);


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
	}



	tuning_positions* load_epd(std::string path);

	double optimal_k(tuning_positions* EPDS);

	double mean_squared_error(tuning_positions* EPDS, double k);

	double changed_error(Parameters p, std::vector<Score> new_values, tuning_positions* EPDS, double k);

	void Tune(Parameters tuning_vars, std::string epd_file, int iterations = 100);

	inline double sigmoid(int eval, double k) {
		return (1.0 / (1.0 + std::pow(10, - (k*double(eval)) / 400.0)));
	}
}


















#endif