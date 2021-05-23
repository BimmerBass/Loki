#ifndef TRAIN_LNN_H
#define TRAIN_LNN_H
#include "../network.h"


namespace Training {

	enum class LOSS_F :int {
		MSE = 0,	/* Mean squared error */
		AAE = 1,	/* Average absolute error */
	};

	class Trainer : private LNN::Network {
	public:
		Trainer(std::string dataset, int epochs, size_t batch_size, LOSS_F loss);

		// Main method. This is responsible for running the training of the network.
		void train_model(std::string saved_network = "");
	private:

		void do_backprop(neuron_t expected_value);
		void update_weights();

		// The following arrays hold all deltas for the hidden layers and the output layer.
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS;
		double OUTPUT_DELTA;
	};

}

#endif