#ifndef TEXEL_H
#define TEXEL_H

#include "uci.h" // Include all of the engine

#include <vector>
#include <fstream>

// Amount of concurrent threads to run when computing the eval error. The higher, the faster the algorithm is.
#define EVAL_THREADS 3

#if EVAL_THREADS < 1 // Make sure we don't use less than one thread
#define EVAL_THREADS 1
#endif

using namespace PSQT;

namespace Texel {
	/*
	Parameters for the sigmoid constant
	*/
	constexpr double k_initial = 1.0;
	constexpr int k_precision = 4; // Amount of digits to use.




	/*
	This structure holds all the necessary data for the parameter, which is its original value, the address (to change it in eval) and its bounds if specified.
	*/
	struct Parameter {

		Parameter (Score* var, Score max_val = Score(INF, INF), Score min_val = Score(-INF, -INF)) {
			variable = var; // Copy the pointer

			max_value = max_val;
			min_value = min_val;

			original_value = *var;
		}

		Score* variable;


		Score max_value;
		Score min_value;


		Score original_value;
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


	tuning_positions* load_epd(std::string path);

	double optimal_k(tuning_positions* EPDS);

	double mean_squared_error(tuning_positions* EPDS, double k);

	double changed_error(Parameters p, std::vector<Score> new_values, tuning_positions* EPDS, double k);

	void Tune(Parameters tuning_vars, std::string epd_file, int iterations = 100);

	// Instead of Österlunds tuning, we use the sigmoid from Ethereal which is based on Eulers constant instead of 10.
	inline double sigmoid(int eval, double k) {
		return (1.0 / (1.0 + std::exp(-k * double(eval))));
	}
}


















#endif