#ifndef TRAIN_LNN_H
#define TRAIN_LNN_H
#include "../network.h"


namespace Training {

	enum class LOSS_F :int {
		MSE = 0,	/* Mean squared error */
		AAE = 1,	/* Average absolute error */
	};


	// Used to hold a datapoint for the trainer
	struct TrainingPosition {
		int8_t network_input[INPUT_SIZE] = { 0 };
		int score = 0;

		void set(int val) {
			memset(network_input, val, sizeof(int8_t) * INPUT_SIZE);
		}
	};



	class Trainer : private LNN::Network {
	public:
		Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss, double lRate = 0.001, double lRate_decay = 0.00001);
		~Trainer();

		// Main method. This is responsible for running the training of the network.
		void train_model(std::string saved_network = "");
	private:
		// The datasets can be very big so the data will be allocated to the heap.
		std::vector<TrainingPosition>* training_data;
		void load_dataset(std::string filepath);

		void do_backprop(neuron_t expected_value);
		void update_weights();

		void take_avg_deltas();
		void clear_deltas();

		// The following arrays hold all deltas for the hidden layers and the output layer.
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS;
		double OUTPUT_DELTA;


		const int epochs;
		const size_t batch_size;
		const LOSS_F loss_function;

		const double learning_rate;
		const double learning_rate_decay;
	};


	// Loss function
	template<LOSS_F F>
	double compute_loss(const std::vector<double>& ai, const std::vector<double>& yi);

	// Here we use the assumption that the ReLU function derivative at x = 0 is 1. In reality this is undefined
	double ReLU_derivate(neuron_t x) {
		return (x < 0) ? 0.0 : 1.0;
	}
}



#endif