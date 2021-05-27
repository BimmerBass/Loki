#ifndef TRAIN_LNN_H
#define TRAIN_LNN_H
#include <ctime>
#include <thread>

#include "../network.h"
#include "helpers.h" // For mathematical functions


namespace Training {

	inline double ReLU(neuron_t x) {
		return std::max(neuron_t(0), x);
	}

	// Here we use the assumption that the ReLU function derivative at x = 0 is 1. In reality this is undefined
	inline double ReLU_derivate(neuron_t x) {
		return (x < 0) ? 0.0 : 1.0;
	}

	// As suggested by Andrew Grant, a sigmoid function is used on the output during training
	constexpr double SIGMOID_BASE = 1.2471; // Found numerically by texel tuning. Maps Loki's eval to a WDL space.

	inline double sigmoid(int x) {
		return 1.0 / (1.0 + std::pow(10, -(SIGMOID_BASE * static_cast<double>(x) / 400.0)));
	}

	inline double sigmoid_derivative(int x) {

		double K = SIGMOID_BASE / 400.0;

		double nominator = std::pow(10.0, -K * double(x)) * K * std::log(10);
		double denominator = std::pow(1.0 + std::pow(10.0, -K * double(x)), 2.0);

		return nominator / denominator;
	}



	/*
	Default hyper parameters
	*/
	constexpr double LEARNING_RATE_DEFAULT = 0.01;
	constexpr double LEARNING_DECAY_DELAULT = 0.0001;

	// Two different loss functions can be used:
	enum class LOSS_F :int {
		MSE = 0,	/* Mean squared error (1/n * sum((a[i] - y[i])^2))*/
		AAE = 1		/* Average absolute error (1/n * sum(|a[i] - y[i]|))*/
	};

	// A structure for holding a training position
	struct TrainingPosition {
		std::array<int8_t, INPUT_SIZE> network_inputs;
		int score;

		void set(int val) {
			network_inputs.fill(val);
			score = val;
		}
	};


	/*
	Each thread will be given a ThreadData object. This holds the gradients, the deltas and the neuron activations.
	NOTE: It is important that ThreadData gets allocated on the heap. If multiple threads are used, it is too big to be on the stack!
	*/
	struct ThreadData {
		// Neuron activations.
		// Note: These should be the weighted sums without activation functions applied. A new forward propagation method is used to account for this.
		std::array<neuron_t, INPUT_SIZE> INPUT_NEURONS;
		std::array<neuron_t, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_NEURONS;
		std::array<neuron_t, HIDDEN_STD_SIZE> SECOND_HIDDEN_NEURONS;
		std::array<neuron_t, HIDDEN_STD_SIZE> THIRD_HIDDEN_NEURONS;
		neuron_t OUTPUT_NEURON;

		void set_input(const std::array<int8_t, INPUT_SIZE>& input);

		// Gradients
		std::array<std::array<double, INPUT_SIZE>, FIRST_HIDDEN_SIZE> INPUT_WEIGHT_GRADIENTS;
		std::array <std::array<double, FIRST_HIDDEN_SIZE>, HIDDEN_STD_SIZE> FIRST_HIDDEN_WEIGHT_GRADIENTS;
		std::array<std::array<double, HIDDEN_STD_SIZE>, HIDDEN_STD_SIZE> SECOND_HIDDEN_WEIGHT_GRADIENTS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_WEIGHT_GRADIENTS;

		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_BIAS_GRADIENTS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_BIAS_GRADIENTS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_BIAS_GRADIENTS;

		// Update the gradients when we find the deltas
		void update_gradients();

		// After having summed up the gradients from a batch, we need to take the average of them.
		void average_gradients(size_t batch_size);
		
		// Set all gradients to zero.
		void clear_gradients();

		/*
		Deltas. Used to hold the local gradient (δ = ∂C/∂z), where z is the weighted sum of the neuron (no activation function)
		We're interested in finding the following two gradients:
		1) ∂C/∂w = ∂C/∂z * ∂z/∂w = δ * ∂z/∂w = δ * a. Where a is the neuron's (which is connected to this one by w) activation in the previous layer.
		2) ∂C/∂b = ∂C/∂z * ∂z/∂b = δ * ∂z/∂b = δ * 1 = δ
		These will be computed using a backpropagation algorithm, and the gradients will then be updated using these.
		*/
		std::array<double, FIRST_HIDDEN_SIZE> FIRST_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> SECOND_HIDDEN_DELTAS;
		std::array<double, HIDDEN_STD_SIZE> THIRD_HIDDEN_DELTAS;
		double OUTPUT_DELTA;

		// Set all deltas to zero
		void clear_deltas();
	};




	/*
	Main class for the training algorithm. This is responsible for giving the threads work and finally updating the weights and biases.
	*/
	class Trainer :private LNN::Network {
	public:
		Trainer(std::string datafile, size_t _epochs, size_t _batch_size, LOSS_F _loss, size_t _threads, 
			double eta_start = LEARNING_RATE_DEFAULT, double eta_decay = LEARNING_DECAY_DELAULT);
		~Trainer();

		void run();

	private:
		// Hyperparameters for the training
		double learning_rate = 0.0;
		const double initial_learning_rate;
		const double learning_rate_decay;

		const size_t epochs;
		const size_t batch_size;
		const size_t thread_count;
		const LOSS_F loss_function;

		// Container and loading method for the training data
		std::vector<TrainingPosition>* training_data;
		void load_dataset(std::string filepath);

		// All threads will run this method. It is the primary optimization method.
		void run_thread(const std::vector<TrainingPosition>& positions, std::vector<double>& outputs, std::vector<double>& expected, int thread_id);

		// Container holding all data for the threads
		std::vector<ThreadData*> thread_data;
		ThreadData* main_thread_data = nullptr;

		// Calculate the average of all gradients in thread_data.
		void compute_average_gradients();

		// Backpropagation.
		void back_propagation(int thread_id, int target_output);

		// Forward propagation
		int forward_propagate(int thread_id);

		// Weight/bias update
		void update_parameters();
	};

}



#endif