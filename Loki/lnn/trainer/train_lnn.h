#ifndef TRAIN_LNN_H
#define TRAIN_LNN_H
#include <ctime>

#include "../network.h"


namespace Training {

	// Here we use the assumption that the ReLU function derivative at x = 0 is 1. In reality this is undefined
	inline double ReLU_derivate(neuron_t x) {
		return (x < 0) ? 0.0 : 1.0;
	}

	// As suggested by Andrew Grant, a sigmoid function is used on the output during training
	constexpr double SIGMOID_BASE = 1.2471; // Found numerically by texel tuning

	inline double sigmoid(int x) {
		return 1.0 / (1.0 + std::pow(10, -(SIGMOID_BASE * static_cast<double>(x) / 400.0)));
	}

	inline double sigmoid_derivative(int x) {
		
		double K = SIGMOID_BASE / 400.0;

		double nominator = std::pow(10.0, -K * double(x)) * K * std::log(10);
		double denominator = std::pow(1.0 + std::pow(10.0, -K * double(x)), 2.0);

		return nominator / denominator;
	}

	enum class LOSS_F :int {
		MSE = 0,	/* Mean squared error */
		AAE = 1,	/* Average absolute error */
	};


	// Used to hold a datapoint for the trainer.
	struct TrainingPosition {
		std::array<int8_t, INPUT_SIZE> network_input;
		int score = 0;

		void set(int val) {
			network_input.fill(val);
		}
	};



	class Trainer : private LNN::Network {
	public:
		Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss, double lRate = 0.000001, double lRate_decay = 0.00001);
		~Trainer();

		// Main method. This is responsible for running the training of the network.
		void train_model(std::string saved_network = "");
	private:
		// The datasets can be very big so the data will be allocated to the heap.
		std::vector<TrainingPosition>* training_data;
		void load_dataset(std::string filepath);

		void do_backprop(double expected_value);
		void update_weights();

		void take_avg_deltas(size_t current_batch_size);
		void clear_deltas();

		void init_parameters();

		// The following arrays hold all deltas for the hidden layers and the output layer.
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS;
		double OUTPUT_DELTA;

		// To do proper backpropagation with batch gradient descent, we need to record the changes in the deltas.
		// The below containers are used for that purpose
		void clear_delta_changes();

		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS_CHANGES;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS_CHANGES;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS_CHANGES;
		double OUTPUT_DELTA_CHANGE = 0;


		const int epochs;
		const size_t batch_size;
		const LOSS_F loss_function;

		const double initial_learning_rate;
		const double learning_rate_decay;

		double learning_rate;

		double calc_learning_rate(int epoch) {
			return initial_learning_rate / (1.0 + learning_rate_decay * double(epoch));
		}
	};


	// Loss function
	template<LOSS_F F>
	double compute_loss(const std::vector<double>& ai, const std::vector<double>& yi);

}



#endif