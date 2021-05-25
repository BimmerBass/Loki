#ifndef TRAIN_LNN_H
#define TRAIN_LNN_H
#include <ctime>
#include <thread>

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


	// Used by each thread to hold the deltas of a training epoch/batch
	struct Deltas {
		// The following arrays hold all deltas for the hidden layers and the output layer.
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS;
		double OUTPUT_DELTA;

		// When updating the deltas, we can't just add to the current deltas since that would ignore the neuron's activation.
		// Therefore we use temporary changes in the deltas.
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS_CHANGES;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS_CHANGES;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS_CHANGES;
		double OUTPUT_DELTA_CHANGE = 0;


		void clear_delta_changes();
		void take_avg_deltas(size_t current_batch_size);
		void clear_deltas();

		// The below are all the layers which will be used to hold the neurons's activations instead of the ones in the network.
		std::array<neuron_t, INPUT_SIZE> INPUT_NEURONS;
		std::array<neuron_t, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_NEURONS;
		std::array<neuron_t, HIDDEN_STD_SIZE> SECOND_HIDDEN_NEURONS;
		std::array<neuron_t, HIDDEN_STD_SIZE> THIRD_HIDDEN_NEURONS;
		neuron_t OUTPUT_NEURON;

		// Set up the position to the input neurons
		void load_input(const std::array<int8_t, INPUT_SIZE>& in) {
			for (int i = 0; i < INPUT_SIZE; i++) {
				INPUT_NEURONS[i] = static_cast<neuron_t>(in[i]);
			}
		}
	};


	class Trainer : private LNN::Network {
	public:
		Trainer(std::string dataset, int _epochs, size_t _batch_size, LOSS_F loss, size_t _threads=1, double lRate = 0.000001, double lRate_decay = 0.00001);
		~Trainer();

		// Main method. This is responsible for running the training of the network.
		void train_model(std::string saved_network = "");
	private:
		// The datasets can be very big so the data will be allocated to the heap.
		std::vector<TrainingPosition>* training_data;
		void load_dataset(std::string filepath);

		// Divide a batc into sub-batches
		void subdivide_batch(std::vector<std::vector<TrainingPosition>>& sub_batches, const std::vector<size_t>& batch, int current);

		// Function that each thread will run.
		void thread_optimizer(const std::vector<TrainingPosition>& positions, std::vector<double>& outputs, std::vector<double>& expected, int thread_id);

		// Kind of overload of the evaluate function in Network. Just uses the thread_id for other neuron containers.
		int thread_evaluator(int thread_id);

		void do_backprop(double expected_value, int thread_id);
		void update_weights();

		void init_parameters();

		// The following delta structs will be declared on heap depending on the amount of threads available.
		std::vector<Deltas*> deltas;
		
		// main_deltas holds the deltas that are actually used to update the weights/biases. Its values are the average of all threads's values.
		Deltas main_deltas;

		// Computes the average of all the threads's deltas and inserts the values in the main_deltas object
		void compute_main_deltas();

		// Clears all deltas in "deltas" and main_deltas
		void clear_all_deltas();

		const size_t threads;

		const int epochs;
		const size_t batch_size;
		const LOSS_F loss_function;

		// LNN uses a decaying learning rate. The below hyperparameters are used for that purpose
		const double initial_learning_rate;
		const double learning_rate_decay;

		double learning_rate;

		double calc_learning_rate(int epoch) {
			return initial_learning_rate / (1.0 + learning_rate_decay * double(epoch));
		}

		/* The functions below are essentially the same as the ones in the Network class, but they use an additional parameter: thread_id */

	};


	// Loss function
	template<LOSS_F F>
	double compute_loss(const std::vector<double>& ai, const std::vector<double>& yi);

}



#endif