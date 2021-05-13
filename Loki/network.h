#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <iostream>
#include <sstream>

#include "search.h"		/* Used for searching all epd positions when generating the training set. Change this to however your engine searches */


namespace Neural {

	/*
	Network architecture definitions
	*/
	constexpr int INPUT_SIZE = 12 * 64;		/* ATM piece placement is the only thing considered */
	constexpr int FIRST_HIDDEN_SIZE = 256;	/* The first hidden layer should have 256 neurons */
	constexpr int HIDDEN_STD_SIZE = 32;		/* The two layers following should have 32 */
	constexpr int HIDDEN_STD_COUNT = 2;		/* Two standard hidden layers */
	constexpr int OUTPUT_SIZE = 1;			/* The output layer should have one neuron ==> A value network*/

	// +/- bound for the output
	constexpr int16_t OUTPUT_BOUND = 30000;


	/*
	Training hyper-parameter definitions
	*/
	constexpr int MINI_BATCH_SIZE = 10000;
	constexpr int PROCESSORS = 8;
	
	// Adam
	constexpr double LRATE = 0.1;
	constexpr double EPSILON = 0.0000000001;
	constexpr double BETA_ONE = 0.9;
	constexpr double BETA_TWO = 0.999;

	// SPSA
	constexpr double C_END = 4.0;
	constexpr double R_END = 0.002;
	constexpr double gamma = 0.101;
	constexpr double alpha = 0.602;


	// This is the generator of the Bernoulli +/- 1 distribution with p = 50%, used in the tuning
	static std::default_random_engine generator;
	static std::bernoulli_distribution distribution(0.5);

	inline double randemacher() {
		return (distribution(generator)) ? 1.0 : -1.0;
	}



	typedef std::array<int16_t*, FIRST_HIDDEN_SIZE + HIDDEN_STD_SIZE * HIDDEN_STD_COUNT
		+ INPUT_SIZE * FIRST_HIDDEN_SIZE + FIRST_HIDDEN_SIZE * HIDDEN_STD_SIZE 
		+ HIDDEN_STD_COUNT * HIDDEN_STD_SIZE * HIDDEN_STD_SIZE + HIDDEN_STD_SIZE> WeightBiasVector;

	typedef std::array<int16_t, INPUT_SIZE + FIRST_HIDDEN_SIZE + HIDDEN_STD_SIZE * HIDDEN_STD_COUNT
		+ INPUT_SIZE * FIRST_HIDDEN_SIZE + FIRST_HIDDEN_SIZE * HIDDEN_STD_SIZE
		+ HIDDEN_STD_COUNT * HIDDEN_STD_SIZE * HIDDEN_STD_SIZE + HIDDEN_STD_SIZE> ThetaVector;

	typedef std::array<int16_t, INPUT_SIZE> NetworkInput;

	struct TrainingPosition {
		TrainingPosition(NetworkInput& i, int16_t s) {
			position = i; score = s;
		}

		NetworkInput position;
		int16_t score;
	};
	typedef std::vector<TrainingPosition> TrainingSet;


	// A neuron has an activation and a bias.
	struct Neuron {
		Neuron(const Neuron& n) {
			activation = n.activation; bias = n.bias;
		}
		Neuron() {
			activation = 0; bias = 0;
		}

		int16_t activation = 0;
		int16_t bias = 0;
	};

	/*
	The network class will hold a network when it gets loaded.
	*/
	class Network {
	public:
		Network(std::string net_file = "");
		Network(const Network& n);

		// This method is responsible for loading a position into the network and calculating the values of the first hidden layer.
		void load_position(std::array<int16_t, INPUT_SIZE>& position);

		// This is the key method of the network. It is responsible for evaluating a position.
		int16_t evaluate();

		// This method is responsible for loading a network from a file. The network will be initialized randomly at first.
		void load_net(std::string file_path);

		// This method is responsible for saving a network to a file.
		void save_net(std::string filename = "");

		// Used by texel SPSA tuning
		std::vector<int16_t*> get_tuning_parameters();

		// Used to either train a new, random, model or tune an already saved one.
		void train_model(std::string epd_file, int iterations = 1000, std::string net_file = "");
	private:

		// Layers
		std::array<Neuron, INPUT_SIZE> INPUT_LAYER;
		std::array<Neuron, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_LAYER;
		std::array<std::array<Neuron, HIDDEN_STD_SIZE>, HIDDEN_STD_COUNT> HIDDEN_LAYERS;
		Neuron output;

		// Weights
		std::array<std::array<int16_t, INPUT_SIZE>, FIRST_HIDDEN_SIZE> INPUT_TO_FIRST;
		std::array<std::array<int16_t, FIRST_HIDDEN_SIZE>, HIDDEN_STD_SIZE> FIRST_TO_HIDDEN;
		std::array<std::array<std::array<int16_t, HIDDEN_STD_SIZE>, HIDDEN_STD_SIZE>, HIDDEN_STD_COUNT> HIDDEN_TO_HIDDEN;
		std::array<int16_t, HIDDEN_STD_SIZE> HIDDEN_TO_OUTPUT;

		/*
		The below methods are used when training
		*/

		double mean_square_error(ThetaVector& new_values, WeightBiasVector& params, TrainingSet& positions);
		void update_weights_and_biases(ThetaVector& new_values, WeightBiasVector& ptr_params);
		void copy_weight_bias_pointers(WeightBiasVector& ptrs);

		void load_epds(TrainingSet& s, std::string epd_file);
		
	};



	inline void seed_random() {
		std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
	}

	// This function simply returns a random number in the range [start, end]
	inline int random_num(int start, int end) {
		seed_random();
		int range = (end - start) + 1;
		return (start + (std::rand() % range));
	}

}









#endif