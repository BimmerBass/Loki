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
	};

}









#endif