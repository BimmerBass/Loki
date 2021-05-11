#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>

namespace Neural {

	/*
	Network architecture definitions
	*/
	constexpr int INPUT_SIZE = 12 * 64;		/* ATM piece placement is the only thing considered */
	constexpr int FIRST_HIDDEN_SIZE = 256;	/* The first hidden layer should have 256 neurons */
	constexpr int HIDDEN_STD_SIZE = 32;		/* The two layers following should have 32 */
	constexpr int HIDDEN_STD_COUNT = 2;		/* Two standard hidden layers */
	constexpr int OUTPUT_SIZE = 1;			/* The output layer should have one neuron ==> A value network*/


	/*
	The network class will hold a network when it gets loaded.
	*/
	class Network {
	public:
		Network();
		~Network();


		// This method is responsible for loading a position into the network
		void load_position(std::array<int16_t, INPUT_SIZE>& position);

		// This is the key method of the network. It is responsible for evaluating a position.
		int16_t evaluate();

	private:
		// A neuron has an activation and a bias.
		struct Neuron {
			int16_t activation = 0;
			int16_t bias = 0;
		};

		// Layers
		std::array<Neuron, INPUT_SIZE> INPUT_LAYER;
		std::array<Neuron, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_LAYER;
		std::array<std::array<Neuron, HIDDEN_STD_SIZE>, HIDDEN_STD_COUNT> HIDDEN_LAYERS;
		Neuron output;

		// Weights
		std::array<std::array<int16_t, INPUT_SIZE>, FIRST_HIDDEN_SIZE> INPUT_TO_FIRST;
		std::array<std::array<int16_t, FIRST_HIDDEN_SIZE>, HIDDEN_STD_SIZE> INPUT_TO_HIDDEN;
		std::array<std::array<int16_t, HIDDEN_STD_SIZE>, HIDDEN_STD_SIZE> HIDDEN_TO_HIDDEN;
		std::array<int16_t, HIDDEN_STD_SIZE> HIDDEN_TO_OUTPUT;
	};

}









#endif