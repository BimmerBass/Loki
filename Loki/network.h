#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include <chrono>

#include "evaluation.h" // For popBit and GameState_t

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

	constexpr int BATCH_SIZE = 75;
	
	constexpr int POPULATION_SIZE = 50;
	constexpr double ALPHA = 0.05;
	constexpr double MUTATION_RATE = 0.01;
	constexpr double LEARNING_RATE = 0.1;

	namespace Training {
		struct TrainingPosition {
			TrainingPosition(std::array<uint64_t, 12> pieces, int32_t eval) {
				pieceBoards = pieces; value = eval;
			}

			std::array<uint64_t, 12> pieceBoards;
			int32_t value;
		};
		typedef std::vector<TrainingPosition> TrainingSet;

		void load_epd(std::string filepath, TrainingSet& set);
	}

	enum class A_FUNC {
		A_NONE = 0,
		SIGMOID = 1
	};

	typedef float neuron_t;

	struct Layer {
		Layer(int n_cnt, int next_layer_len, A_FUNC a_function, bool is_input = false);
		~Layer();
		Layer(const Layer& l);

		neuron_t* neurons;

		neuron_t** weights;
		neuron_t* biases;

		double* deltas;


		int neuron_count;
		int weight_count;

		int next_layer_length;

		A_FUNC activation_function;

		// Set all neurons to this value
		void set(neuron_t val);
	};



	class NeuralNet {
	public:
		// Our constructor takes a vector with the number of hidden layers as elements and each element corresponding to the hidden layer's length.
		NeuralNet(std::vector<int> arch);

		// This is the feedforward method for the network
		int32_t evaluate();

		// This method is responsible for loading a position. This is slow and should not be used when making and un-making moves. Only initialization
		// Note: The ordering of the bitboards array should be: WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK
		void load_position(std::array<uint64_t, 12>& bitboards);

		void train_model(int iterations);

		//void save_net();
		//void load_net(std::string file);
	private:
		std::vector<Layer> layers;

		void copy_weights_and_biases(std::vector<neuron_t*>& v);

		void change_parameters(std::vector<neuron_t>& new_values, std::vector<neuron_t*>& parameters);
		double compute_error(int start, std::vector<neuron_t>& new_values, std::vector<neuron_t*>& parameters, Training::TrainingSet& set);

		void back_propagate(int32_t expected_output);
		void update_weights();
		void clear_deltas();
	};


	// Selection in the genetic algorithm
	void pick_best(std::vector<int>& index_vector, std::vector<double> losses);


	// Activation function
	//template<A_FUNC A>
	//int32_t activation_function(int32_t x);

	inline int calculate_index(int pce, int sq) {
		assert(pce >= 0 && pce <= 11);
		assert(sq >= 0 && sq <= 63);

		return (pce * 64) + sq;
	}


	// Seed the RNG with the time since epoch.
	inline void seed_random() {
		std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
	}

	// This function simply returns a random number in the range [start, end]
	inline int random_num(int start, int end) {
		seed_random();
		int range = (end - start) + 1;
		return (start + (std::rand() % range));
	}


	// This is the generator of the Bernoulli +/- 1 distribution with p = 50%, used in the tuning
	static std::default_random_engine generator;
	static std::bernoulli_distribution distribution(0.5);

	inline double randemacher() {
		return (distribution(generator)) ? 1 : -1;
	}

	inline neuron_t sigmoid(neuron_t x) {
		//return 1.0 / (1.0 + std::exp(-x));
		return std::tanh(x);
	}

	inline neuron_t sigmoid_derivative(neuron_t x) {
		return std::pow(1.0 / std::cosh(x), 2.0);
	}

}






#endif