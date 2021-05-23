#include "train_lnn.h"



namespace Training {

	/*
	
	Loss function. Here we use two different types:

	MSE: Mean squared error (1/n * sum((ai - yi)^2))
	AAE: Absolute average error = (1/n * sum(|ai - yi|))
	
	*/

	template<>
	double compute_loss<LOSS_F::MSE>(const std::vector<double>& ai, const std::vector<double>& yi) {
		assert(ai.size() == yi.size());
		uint64_t sum = 0;

		for (int i = 0; i < ai.size(); i++) {
			sum += std::pow(ai[i] - yi[i], 2.0);
		}

		return static_cast<double>(sum) / ai.size();
	}

	template<>
	double compute_loss<LOSS_F::AAE>(const std::vector<double>& ai, const std::vector<double>& yi) {
		assert(ai.size() == yi.size());
		uint64_t sum = 0;

		for (int i = 0; i < ai.size(); i++) {
			sum += abs(ai[i] - yi[i]);
		}

		return static_cast<double>(sum) / ai.size();
	}




}